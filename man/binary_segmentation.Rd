% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/changepoints.R
\docType{methods}
\name{binary_segmentation}
\alias{binary_segmentation}
\alias{binary_segmentation,changepointsMod-method}
\title{Multiple change-point method.}
\usage{
binary_segmentation(object, method, thresh = 0, buff = 100,
  method_params = list())

\S4method{binary_segmentation}{changepointsMod}(object, method, thresh = 0,
  buff = 100, method_params = list())
}
\arguments{
\item{object}{Corresponding \code{changepointsMod} class.}

\item{method}{changepointHD method for finding single change-point.}

\item{thresh}{Stopping threshold for cost comparison.}

\item{buff}{Distance from edge of sample to be maintained during search.}

\item{method_params}{List of additional parameters for \code{method}.}
}
\value{
An updated version of the change-point model.  The update will effect:
        1) An estimate for the current set of change-points.  2) The \code{mod_list},
        this will correspond to all the active single change-point models
        generated during the binary-segmentation procedure.  Acitve models
        correspond to models that have not been superseded by more granular
        models.  3) The \code{mod_range}, this corresponds to the range of
        observations covered by each model.  It can be used to determine which
        models are active.
}
\description{
Estimates multiple change-points using the binary-segmentation
             method.  This does a breadth first search and uses the specified
             single change-point method for each sub-search.
}
\examples{
set.seed(334)

mcp_data = read.table(system.file("extdata", "mcp.txt", package="changepointsHD"))
mcp_data = as.matrix(mcp_data)

# prox gradient black-box method
cov_est = cov(mcp_data)
init = solve(cov_est)
res_map = prox_gradient_mapping(mcp_data, init, 0.1, 0.99, 0.1, 100, 1e-20)

# prox gradient black-box ll
res_ll = prox_gradient_ll(mcp_data, res_map, 0.1)

prox_gradient_params=list()
prox_gradient_params$update_w = 0.1
prox_gradient_params$update_change = 0.99
prox_gradient_params$regularizer = 0.1
prox_gradient_params$max_iter = 1
prox_gradient_params$tol = 1e-5

prox_gradient_ll_params=list()
prox_gradient_ll_params$regularizer = 0.1

simulated_annealing_params = list()
simulated_annealing_params$buff=10

changepoints_mod = changepointsMod(bbmod=prox_gradient_mapping,
                                 log_likelihood=prox_gradient_ll,
                                 bbmod_params=prox_gradient_params,
                                 ll_params=prox_gradient_ll_params,
                                 part_values=list(init, init),
                                 data=list(mcp_data))

changepoints_mod = binary_segmentation(changepoints_mod, method=simulated_annealing,
                                       thresh=0, buff=10,
                                       method_params=simulated_annealing_params)

}
\author{
\packageMaintainer{changepointsHD}
}
