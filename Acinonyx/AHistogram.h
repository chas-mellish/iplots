/*
 *  AHistogram.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/23/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_HISTOGRAM_H
#define A_HISTOGRAM_H

#include "AStatVisual.h"
#include "APlot.h"

#define UNDERFLOW_BIN (_bins)
#define OVERFLOW_BIN (_bins + 1)
#define NA_BIN (_bins + 2)

class ABinning : public AObject {
protected:
	ADataVector *x;
	unsigned int *_counts;
	vsize_t _bins, _maxCt, _allocBins;
	double _anchor, _binw;
	vsize_t *_groups;
	
public:
	ABinning(ADataVector *data, vsize_t n_bins, double anchor_val, double binw_val, bool alloc_groups = false) : x(data), _bins(n_bins), _anchor(anchor_val), _binw(binw_val), _maxCt(0), _groups(alloc_groups ? ((vsize_t*)calloc(sizeof(vsize_t), data->length())) : NULL) {
		_allocBins = _bins + 8;
		_counts = (unsigned int *) calloc(sizeof(unsigned int), _allocBins);
		if (x) x->retain();
		updateCounts();
		OCLASS(ABinning);
	}
	
	virtual ~ABinning() {
		x->release();
		free(_counts);
		if (_groups) free(_groups);
		DCLASS(ABinning)
	}
	
	void updateCounts() {
		memset(_counts, 0, sizeof(unsigned int) * (_bins + 3));
		if (!x) return;
		const double *d = x->asDoubles(), maxv = _anchor + ((double) _bins) * _binw;
		vsize_t n = x->length();
		for (vsize_t i = 0; i < n; i++) {
			// FIXME: we need IS_NA(X)!
			vsize_t this_bin = NA_BIN;
			if (d[i] < _anchor) this_bin = UNDERFLOW_BIN;
			else if (d[i] >= maxv) this_bin = OVERFLOW_BIN;
			else this_bin = (vsize_t) ((d[i] - _anchor) / _binw);
			_counts[this_bin]++;
			if (_groups) _groups[i] = this_bin;
		}
		_maxCt = _counts[0];
		for (vsize_t i = 1; i < _bins; i++) if (_counts[i] > _maxCt) _maxCt = _counts[i];
	}

	unsigned int count(vsize_t bin) {
		return (bin >= _bins) ? 0 : _counts[bin];
	}
	
	unsigned int underflow() { return _counts[UNDERFLOW_BIN]; }
	unsigned int overflow() { return _counts[OVERFLOW_BIN]; }
	unsigned int nas() { return _counts[NA_BIN]; }
	unsigned int maxCount() { return _maxCt; } // excluding special bins
	
	void setAnchor(double a) { _anchor = a; }
	void setBinWidth(double w) { _binw = w; }
	void setBins(vsize_t n) {
		ALog("%s: setBins(%d) - _bins=%d, _allocBins=%d", describe(), n, _bins, _allocBins);
		_bins = n; 
		if (_bins + 3 > _allocBins) {
			_allocBins *= 2; if (_allocBins < _bins + 3) _allocBins = _bins + 8;
			vsize_t *new_counts = (vsize_t*) realloc(_counts, _allocBins * sizeof(vsize_t));
			AMEM(new_counts);
			ALog(" - realloc to %d for %d bins (%p)", _allocBins, _bins, new_counts);
			_counts = new_counts;
		}
		updateCounts();
	}
	
	vsize_t bins() { return _bins; }
	double anchor() { return _anchor; }
	double binWidth() { return _binw; }
	vsize_t *groups() { return _groups; }
};

class AHistogram : public APlot {
protected:
	ADataVector *data;
	AXAxis *xa;
	AYAxis *ya;
	ABinning *bin;
	bool spines;
	
public:
	AHistogram(AContainer *parent, ARect frame, int flags, ADataVector *x) : APlot(parent, frame, flags), bin(new ABinning(x, 11, x->range().begin, x->range().length / 11.0, true)), spines(false) {
		nScales = 2;
		data = x; x->retain();
		marker = x->marker();
		if (marker) {
			marker->retain();
			marker->add(this);
		}
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		_scales[0] = new AScale(x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		_scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), AMkDataRange(0, bin->maxCount()));
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, _scales[0]);
		add(*xa);
		xa->release();
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, _scales[1]);
		add(*ya);
		ya->release();
		updatePrimitives(true);
		OCLASS(AHistogram)		
	}
	
	virtual ~AHistogram() {
		data->release();
		bin->release();
		DCLASS(AHistogram)
	}
	
	void updatePrimitives(bool create=false) {
		vsize_t n = data->length();
		ALog("%s: n = %d", describe(), n);
		vsize_t bins = bin->bins();
		if (pps && pps->length() != bins) {
			pps->release();
			pps = NULL;
			create = true;
		}
		if (!pps)
			pps = new ASettableObjectVector(bins);
		AFloat  bottom = _scales[1]->position(0);
		if (!spines) { // regular axis
			double dpos = bin->anchor();
			for(vsize_t i = 0; i < bins; i++) {
				vsize_t c = bin->count(i);
				AFloat left = _scales[0]->position(dpos); dpos += bin->binWidth();
				ARect r = AMkRect(left, bottom, _scales[0]->position(dpos) - left, _scales[1]->position(c) - bottom);
				if (create) {
					ABarStatVisual *b = new ABarStatVisual(this, r, Up, marker, bin->groups(), n, i, false, false);
					((ASettableObjectVector*)pps)->replaceObjectAt(i, b);
					b->release(); // we passed the ownership to pps
				} else ((ABarStatVisual*)((ASettableObjectVector*)pps)->objectAt(i))->setRect(r);
			}
		} else { // count-based axis
			double dpos = 0.0, height = _scales[1]->position(1) - bottom;
			for(vsize_t i = 0; i < bins; i++) {
				vsize_t c = bin->count(i);
				AFloat left = _scales[0]->position(dpos); dpos += c;
				ARect r = AMkRect(left, bottom, _scales[0]->position(dpos) - left, height);
				if (create) {
					ABarStatVisual *b = new ABarStatVisual(this, r, Up, marker, bin->groups(), n, i, false, false);
					((ASettableObjectVector*)pps)->replaceObjectAt(i, b);
					b->release(); // we passed the ownership to pps
				} else ((ABarStatVisual*)((ASettableObjectVector*)pps)->objectAt(i))->setRect(r);
			}
		}		
	}

	void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		_scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));
		
		// ya->setHidden(spines);
		
		if (spines) {
			_scales[0]->setDataRange(AMkDataRange(0, data->length()));
			_scales[1]->setDataRange(AMkDataRange(0, 1));
		} else {
			_scales[0]->setDataRange(data->range());
			_scales[1]->setDataRange(AMkDataRange(0, bin->maxCount()));
		}
		updatePrimitives();
	}
	
	virtual bool keyDown(AEvent e) {
		switch (e.key) {
			case 1: spines = !spines; update(); redraw(); break;
			case KEY_UP: if (bin->bins() > 1) { bin->setBinWidth(bin->binWidth() * (double)bin->bins() / ((double)bin->bins() - 1.0)); bin->setBins(bin->bins() - 1); updatePrimitives();  redraw(); }; break;
			case KEY_DOWN: bin->setBinWidth(bin->binWidth() * (double)bin->bins() / ((double)bin->bins() + 1.0)); bin->setBins(bin->bins() + 1); updatePrimitives(); redraw(); break;
			default:
				return false;
		}
		return true;
	}	
};

#endif
