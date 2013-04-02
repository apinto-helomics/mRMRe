\name{adjacencyMatrix}
\Rdversion{1.1}
\docType{methods}
\alias{adjacencyMatrix}
\alias{adjacencyMatrixSum}
\alias{adjacencyMatrix,mRMRe.Network-method}
\alias{adjacencyMatrixSum,mRMRe.Network-method}
\title{
	Accessor function for the 'adjacencyMatrix' information in a mRMRe.Network object.
	
}
\description{
	The adjency matrix is a directed matrix of 0's and 1's indicating if there is a link between features.
}
\usage{
\S4method{adjacencyMatrix}{mRMRe.Network}(object)
}
\arguments{
  \item{object}{a \code{mRMRe.Network} object.}
}
\author{
Nicolas De Jay, Simon Papillon-Cavanagh, Benjamin Haibe-Kains
}
\examples{
data(adjacencyMatrix)
## maybe str(adjacencyMatrix) ; plot(adjacencyMatrix) ...
}
\keyword{datasets}