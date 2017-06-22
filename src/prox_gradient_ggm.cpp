// [[Rcpp::depends(RcppArmadillo)]]

#include <RcppArmadillo.h>
#include "prox_gradient_ggm.h"

using namespace std;
using namespace Rcpp;
using namespace arma;


// proximal gradient black box model
//' @name prox_gradient_mapping
//'
//' @title Performs the proximal-gradient mapping operation to
//'        estimate a regularized version of the inverse cov
//'        matrix
//'
//' @description Performs the proximal-gradient mapping operation to
//'              estimate a regularized version of the inverse cov
//'              matrix
//'
//' @param data N x P matrix corresponding to the raw data
//' @param theta_start initial value for theta estimate
//' @param update_w step size for prox-gradient
//' @param update_change proportion of update_w to keep when
//'        the algorithm fails to successfully estimate theta
//' @param regularizer regularizing constant, lambda
//' @param max_iter number of mapping iterations
//' @param tol tolerance at which the algorithm stops running
//'
//' @return theta estimate
//'
//' @author Leland Bybee \email{leland.bybee@@gmail.com}
// [[Rcpp::export]]
arma::mat prox_gradient_mapping(arma::mat data, arma::mat theta_start,
                                double update_w, double update_change,
                                double regularizer, int max_iter, double tol){
    /* Produces the regularized estimation of the covariance matrix using
     * the proximal gradient procedure described in
     *
     * http://dept.stat.lsa.umich.edu/~yvesa/sto_prox.pdf
     *
     * Parameters
     * ----------
     *
     *  data : mat
     *      The data for the current tau values, should
     *      be N x P.
     *  theta_start : mat
     *      The starting value for theta, should be P x P.
     *  update_w : double
     *      The weight for the update to theta, gamma in the source paper.
     *  update_change : double
     *      Change to update_w when estimation procedure fails.  New
     *      update_w *= update_change.
     *      The amount by which update_w shrinks with each failed run
     *  regularizer : double
     *      Regularizing constant, lamb in source paper.
     *  max_iter : int
     *      Limits the maximum number iterations.
     *  tol : double
     *      The tolerance at which updates are stopped.
     *
     * Returns
     * -------
     *
     *  theta_p : mat
     *      current estimate for theta.
     */

    int N = data.n_rows;
    int P = data.n_cols;

    mat cov_est = cov(data);

    // TODO may not need to fill theta_p and inv_theta with 0s
    // proposed theta estimate
    mat theta_p = eye<mat>(P, P);
    // current theta estimate
    mat theta_k = theta_start;
    mat inv_theta = eye<mat>(P, P);

    int i = 0;

    float delta_norm = norm(theta_k - theta_p) / norm(theta_k);

    bool state = false;

    while (state and i < max_iter) {

        try {
            inv_theta = inv(theta_k);
            theta_p = theta_k - update_w * (cov_est - inv_theta);
            for (int j = 0; j < P; j++) {
                for (int k = 0; k < P; k++) {
                    if (theta_p(j,k) <= -regularizer) {
                        theta_p(j,k) += regularizer;
                    }
                    else if (theta_p(j,k) >= regularizer) {
                        theta_p(j,k) -= regularizer;
                    }
                    else {
                        theta_p(j,k) = 0;
                    }
                }
            }
        }
        // TODO add correct error here
        catch(...) {
            update_w *= update_change;
            theta_k = theta_start;
        }
        delta_norm = norm(theta_k - theta_p) / norm(theta_k);
        theta_k = theta_p;
        if (delta_norm < tol) {
            state = false;
        }
        i += 1;
    }

    return(theta_p);
}


// proximal gradient black box model log likelihood
//' @name prox_gradient_ll
//'
//' @title estimates the log-likeihood for the corresponding
//'        theta and data set
//'
//' @description estimates the log-likeihood for the corresponding
//'              theta and data set
//'
//' @param data N x P matrix corresponding to the raw data
//' @param theta_i estimate for theta
//' @param regularizer regularizing constant, lambda
//'
//' @return log-likelihood
//'
//' @author Leland Bybee \email{leland.bybee@@gmail.com}
// [[Rcpp::export]]
double prox_gradient_ll(arma::mat data, arma::mat theta_i,
                        double regularizer) {
    /* Generates the log-likelihood for the specified theta and
     * tau values
     *
     * Parameters
     * ----------
     *
     *  data : mat
     *      N x P matrix containing the data used for estimation
     *  theta_i : mat
     *      P x P inverse cov estimate for first partition
     *  regularizer : double
     *      regularizing constant
     *
     * Returns
     * -------
     *
     *  double corresponding to log-likelihood
     *
     * Notes
     * -----
     *
     *  - {N_tau / 2 * [tr(theta_i.T * S_i) - logdet(theta_i)]
     *     + lambda_tau * || theta_i || / 2}
     *  We take the negative here because we are minimizing
     *  this. TODO confirm that everything lines up
     *
     *  Also, we divide the regularizer by 2 so that we can
     *  include one in both likelihood (and not have to
     *  have this as external code).
     */

    int N = data.n_rows;
    int P = data.n_cols;

    // new
    mat S = cov(data);

    mat TdS = theta_i % S;
    double tr_TdS = trace(TdS);

    double sign;
    double det; log_det(det, sign, theta_i);

    double ll = N * 0.5 * (-det + tr_TdS);
    ll += regularizer * sqrt(log(P) / (N)) * norm(theta_i, 1) * 0.5;

    return -ll;
}