\name{sampleStrata}
\Rdversion{1.1}
\docType{methods}
\alias{sampleStrata}
\alias{sampleStrata<-}
\alias{sampleStrata,mMRRe.Data-method}
\alias{sampleStrata<-,mMRRe.Data,numeric-method}

\title{
	Accessor function for the 'sampleStrata' information in a mRMRe.Data object
}
\description{
	The sampleStrata vector consists of a sampling stratification that will be used in computing mutual information
	between features.  If known batch effects or sample stratification is present between samples, identify such subsets
	using this.
}
\usage{
\S4method{sampleStrata}{mRMRe.Data}(object)
\S4method{sampleStrata}{mRMRe.Data, methods}(object) <- value
}
\arguments{
  \item{object}{a \code{mRMRe.Data} object.}
  \item{value}{a factor vector identifying the stratification of samples.}
}
\author{
Nicolas De Jay, Simon Papillon-Cavanagh, Benjamin Haibe-Kains
}
\examples{
data(sampleStrata)
## maybe str(sampleStrata) ; plot(sampleStrata) ...
}
\keyword{datasets}