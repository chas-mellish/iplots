/*
 *  ARVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/26/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_R_VECTOR_H__
#define A_R_VECTOR_H__

#include "AVector.h"
#include "RObject.h"

class AContainsRObject {
protected:
	RObject *ro;
public:
	AContainsRObject(SEXP data) {
		ro = new RObject(data);
	}

	AContainsRObject(RObject *robj) {
		ro = robj;
		robj->retain();
	}
	
	virtual ~AContainsRObject() {
		if (ro) ro->release();
	}
	
	RObject *object() { return ro; }
	SEXP value() { return ro ? ro->value() : R_NilValue; }
};

class ARDoubleVector : public ADoubleVector, AContainsRObject {
public:
	ARDoubleVector(AMarker *m, SEXP data) : ADoubleVector(m, REAL(data), LENGTH(data), false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARDoubleVector)
	}

	ARDoubleVector(AMarker *m, RObject *o) : ADoubleVector(m, REAL(o->value()), LENGTH(o->value()), false), AContainsRObject(o) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARDoubleVector)
	}
};

class ARIntVector : public AIntVector, AContainsRObject {
public:
	ARIntVector(AMarker *m, SEXP data) : AIntVector(m, INTEGER(data), LENGTH(data), false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARIntVector)
	}

	ARIntVector(AMarker *m, RObject *o) : AIntVector(m, INTEGER(o->value()), LENGTH(o->value()), false), AContainsRObject(o) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARIntVector)
	}
};

class ARFactorVector : public AFactorVector, AContainsRObject {
public:
	ARFactorVector(AMarker *m, SEXP data) : AFactorVector(m, INTEGER(data), LENGTH(data), NULL, 0, false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		SEXP sl = Rf_getAttrib(data, R_LevelsSymbol);
		if (TYPEOF(sl) == STRSXP) {
			_levels = LENGTH(sl);
			_names = (char**) malloc(sizeof(char*) * _levels);
			AMEM(_names);
			for (vsize_t i = 0; i < _levels; i++)
				_names[i] = (char*) CHAR(STRING_ELT(sl, i));
		}
		OCLASS(ARFactorVector)
	}
};

#endif