\name{move}
\alias{move}
\alias{move.iVisual}
\alias{move.iWindow}
\title{
Move an object
}
\description{
\code{move} is a generic function to move an object.
}
\usage{
move(x, ...)
\method{move}{iVisual}(x, xpos, ypos, redraw = TRUE, ...)
\method{move}{iWindow}(x, xpos, ypos, ...)
}
\arguments{
  \item{x}{object to move}
  \item{\dots}{argument specifying the nature of the move as applicable
    to the object and passed to its methods}
  \item{xpos}{x position}
  \item{ypos}{y position}
  \item{redraw}{if \code{TRUE} then \code{\link{redraw}}() is called
    implicitly after the move}
}
%\details{
%}
\note{\code{iVisual} includes any visual object including plots,
  primitives and other graphics objects. The coordinates are interpreted
  with respect to the enclosing window (\code{iVisual}) or screen
  (\code{iWindow}).
}
\value{
  \code{x} updated to reflect the move
}
%\seealso{
%}
%\examples{
%}
\keyword{iplot}
