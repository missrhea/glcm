#include <RcppArmadillo.h>
using namespace arma;

// Define a pointer to a texture function that will be used to map selected 
// co-occurrence statistics to the below texture calculation functions.
typedef double (*pfunc)(mat, mat, mat, double, double);

double text_mean(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "text_mean" << std::endl;
    // Defined as in Lu and Batistella, 2005, page 252
    return(mr);
}

double text_variance(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "variance " << std::endl;
    // Defined as in Haralick, 1973, page 619 (equation 4)
    return(accu(pij % square(imat - mr)));
}

double text_covariance(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "covariance" << std::endl;
    // Defined as in Pratt, 2007, page 540
    return(accu((imat - mr) % (jmat - mc) % pij));
}

double text_homogeneity(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "homogeneity" << std::endl;
    // Defined as in Gonzalez and Woods, 2009, page 832
    return(accu(pij / (1 + abs(imat - jmat))));
}

double text_contrast(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "contrast" << std::endl;
    // Defined as in Gonzalez and Woods, 2009, page 832
    return(accu(square((imat - jmat)) % pij));
}

double text_dissimilarity(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "dissimilarity" << std::endl;
    //TODO: Find source for dissimilarity
    return(accu(pij % abs(imat - jmat)));
}

double text_entropy(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "entropy" << std::endl;
    // Defined as in Haralick, 1973, page 619 (equation 9)
    return(-accu(pij % log(pij + .00000001)));
}

double text_second_moment(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "second_moment" << std::endl;
    // Defined as in Haralick, 1973, page 619
    return(accu(square(pij)));
}

double text_correlation(mat pij, mat imat, mat jmat, double mr, double mc) {
    //Rcpp::Rcout << "correlation" << std::endl;
    // Defined as in Gonzalez and Woods, 2009, page 832
    // Calculate sigr and sigc (measures of row and column std deviation)
    double sigc=0, sigr=0;
    sigr = sqrt(sum(trans(square(linspace<vec>(1, pij.n_rows, pij.n_rows) - mr)) % sum(pij, 0)));
    sigc = sqrt(sum(square(linspace<vec>(1, pij.n_cols, pij.n_cols) - mc) % sum(pij, 1)));
    //return(accu(((imat - mr) % (jmat - mc) % pij) / (sigr * sigc)));
    return(accu((imat % jmat % pij - mr * mc) / (sigr * sigc)));
}

//' Calculates a glcm texture for use in the glcm.R script
//'
//' This function is called by the \code{\link{glcm}} function. It is 
//' not intended to be used directly.
//'
//' @export
//' @param Rast a matrix containing the pixels to be used in the texture 
//' calculation
//' @param statistics a list of strings naming the texture statistics to 
//' calculate
//' @param base_indices a list of column-major indices giving the cells
//' included in the base image
//' @param offset_indices base_indices a list of column-major indices giving 
//' the cells ' included in the offset image
//' @param n_grey number of grey levels to use in texture calculation
//' @return a list of length equal to the length of the \code{statistics} input 
//' parameter, containing the selected textures measures
//' @references
//' Sarah Goslee. Analyzing Remote Sensing Data in {R}: The {landsat} Package.  
//' Journal of Statistical Software, 2011, 43:4, pg 1--25.  
//' http://www.jstatsoft.org/v43/i04/
// [[Rcpp::export]]
Rcpp::NumericVector calc_texture(arma::mat Rast,
        Rcpp::CharacterVector statistics, arma::vec base_indices,
        arma::vec offset_indices, int n_grey) {
    mat G(n_grey, n_grey, fill::zeros);
    mat pij(n_grey, n_grey, fill::zeros);
    mat imat(n_grey, n_grey, fill::zeros);
    mat jmat(n_grey, n_grey, fill::zeros);
    Rcpp::NumericVector textures(statistics.size());
    double mr=0, mc=0;

    // Convert R 1 based indices C++ 0 based indices
    base_indices = base_indices - 1;
    offset_indices = offset_indices - 1;

    std::map<std::string, double (*)(mat, mat, mat, double, double)> stat_func_map;
    stat_func_map["mean"] = text_mean;
    stat_func_map["variance"] = text_variance;
    stat_func_map["covariance"] = text_covariance;
    stat_func_map["homogeneity"] = text_homogeneity;
    stat_func_map["contrast"] = text_contrast;
    stat_func_map["dissimilarity"] = text_dissimilarity;
    stat_func_map["entropy"] = text_entropy;
    stat_func_map["second_moment"] = text_second_moment;
    stat_func_map["correlation"] = text_correlation;

    for(unsigned i=0; i < offset_indices.size(); i++) {
        // Subtract one from the below indices to correct for row and col 
        // indices starting at 0 in C++ versus 1 in R.
        G(Rast(base_indices(i)) - 1, Rast(offset_indices(i)) - 1)++;
    }
    pij = G / accu(G);

    // Make a matrix of i's and a matrix of j's to be used in the below matrix 
    // calculations. These matrices are the same shape as pij with the entries 
    // equal to the i indices of each cell (for the imat matrix, which is 
    // indexed over the rows) or the j indices of each cell (for the jmat 
    // matrix, which is indexed over the columns). Note that linspace<mat> 
    // makes a column vector.
    imat = repmat(linspace<vec>(1, G.n_rows, G.n_rows), 1, G.n_cols);
    jmat = trans(imat);
    // Calculate mr and mc (forms of col and row means), see Gonzalez and 
    // Woods, 2009, page 832
    mr = sum(trans(linspace<vec>(1, G.n_rows, G.n_rows)) % sum(pij, 0));
    mc = sum(linspace<vec>(1, G.n_cols, G.n_cols) % sum(pij, 1));

    // Loop over the selected statistics, using the stat_func_map map to map 
    // each selected statistic to the appropriate texture function.
    for(signed i=0; i < statistics.size(); i++) {
        pfunc f = stat_func_map[Rcpp::as<std::string>(statistics(i))];
        textures(i) = (*f)(pij, imat, jmat, mr, mc);
    }

    return(textures);
}

//' Calculates a glcm texture for use in the glcm.R script
//'
//' This function is called by the \code{\link{glcm}} function. It is 
//' not intended to be used directly.
//'
//' @export
//' @param rast a matrix containing the pixels to be used in the texture 
//' calculation
//' @param statistics a list of strings naming the texture statistics to 
//' calculate
//' @param n_grey number of grey levels to use in texture calculation
//' @param window_dims 2 element list with row and column dimensions of the
//' texture window
//' @param shift a length 2 vector with the number of cells to shift when
//' computing co-ocurrency matrices
//' @return a list of length equal to the length of the \code{statistics} input 
//' parameter, containing the selected textures measures
//' @references
//' Sarah Goslee. Analyzing Remote Sensing Data in {R}: The {landsat} Package.  
//' Journal of Statistical Software, 2011, 43:4, pg 1--25.  
//' http://www.jstatsoft.org/v43/i04/
// [[Rcpp::export]]
arma::cube calc_texture_full_image(arma::mat rast,
        Rcpp::CharacterVector statistics, int n_grey,
        arma::vec window_dims, arma::vec shift) {
    mat pij(n_grey, n_grey);
    mat imat(n_grey, n_grey);
    mat jmat(n_grey, n_grey);
    mat base_window(window_dims(0), window_dims(1));
    mat offset_window(window_dims(0), window_dims(1));
    mat G(n_grey, n_grey, fill::zeros);
    vec base_ul(2), offset_ul(2), center_coord(2);
    double mr, mc, base_window_mean;
    // textures cube will hold the calculated texture statistics
    cube textures(rast.n_rows, rast.n_cols, statistics.size(), fill::zeros);

    std::map<std::string, double (*)(mat, mat, mat, double, double)> stat_func_map;
    stat_func_map["mean"] = text_mean;
    stat_func_map["variance"] = text_variance;
    stat_func_map["covariance"] = text_covariance;
    stat_func_map["homogeneity"] = text_homogeneity;
    stat_func_map["contrast"] = text_contrast;
    stat_func_map["dissimilarity"] = text_dissimilarity;
    stat_func_map["entropy"] = text_entropy;
    stat_func_map["second_moment"] = text_second_moment;
    stat_func_map["correlation"] = text_correlation;

    // Calculate the base upper left (ul) coords and offset upper left coords 
    // as row, column with zero based indices.
    base_ul = vec("0 0");
    if (shift[0] < 0) {
        base_ul[0] = base_ul[0] + abs(shift[0]);
    }
    if (shift[1] < 0) {
        base_ul[1] = base_ul[1] + abs(shift[1]);
    }
    offset_ul = base_ul + shift;
    center_coord = base_ul + floor(window_dims / 2);
     
    // Rcpp::Rcout << "round(window_dims / 2): " << floor(window_dims / 2) << std::endl;
    // Rcpp::Rcout << "base_ul: " << base_ul[0] << ", " << base_ul[1] << std::endl;
    // Rcpp::Rcout << "offset_ul: " << offset_ul[0] << ", " << offset_ul[1] << std::endl;
    // Rcpp::Rcout << "center_coord: " << center_coord[0] << ", " << center_coord[1] << std::endl;

    // Make a matrix of i's and a matrix of j's to be used in the below matrix 
    // calculations. These matrices are the same shape as pij with the entries 
    // equal to the i indices of each cell (for the imat matrix, which is 
    // indexed over the rows) or the j indices of each cell (for the jmat 
    // matrix, which is indexed over the columns). Note that linspace<mat> 
    // makes a column vector.
    imat = repmat(linspace<vec>(1, G.n_rows, G.n_rows), 1, G.n_cols);
    jmat = trans(imat);

    for(unsigned row=0; row < (rast.n_rows - abs(shift(0)) - ceil(window_dims(0)/2)); row++) {
        // if (row %250 == 0 ) {
        //     Rcpp::Rcout << "Row: " << row << std::endl;
        // }
        for(unsigned col=0; col < (rast.n_cols - abs(shift(1)) - ceil(window_dims(1)/2)); col++) {
            base_window = rast.submat(row + base_ul(0),
                                      col + base_ul(1),
                                      row + base_ul(0) + window_dims(0) - 1,
                                      col + base_ul(1) + window_dims(1) - 1);
            offset_window = rast.submat(row + offset_ul(0),
                                        col + offset_ul(1),
                                        row + offset_ul(0) + window_dims(0) - 1,
                                        col + offset_ul(1) + window_dims(1) - 1);

            G.fill(0);
            for(unsigned i=0; i < base_window.n_elem; i++) {
                // Subtract one from the below indices to correct for row and col 
                // indices starting at 0 in C++ versus 1 in R.
                G(base_window(i) - 1, offset_window(i) - 1)++;
            }
            pij = G / accu(G);

            // Calculate mr and mc (forms of col and row means), see Gonzalez 
            // and Woods, 2009, page 832
            mr = sum(linspace<vec>(1, G.n_cols, G.n_cols) % sum(pij, 1));
            mc = sum(trans(linspace<vec>(1, G.n_rows, G.n_rows)) % sum(pij, 0));

            base_window_mean = accu(base_window) / base_window.size();

            // Rcpp::Rcout << "mr vec" << std::endl;
            // trans(linspace<vec>(1, G.n_rows, G.n_rows)).print();
            // Rcpp::Rcout << "mc vec" << std::endl;
            // linspace<vec>(1, G.n_cols, G.n_cols).print();
            // Rcpp::Rcout << "imat" << std::endl;
            // imat.print();
            // Rcpp::Rcout << "jmat" << std::endl;
            // jmat.print();

            // Loop over the selected statistics, using the stat_func_map map to map 
            // each selected statistic to the appropriate texture function.
            for(signed i=0; i < statistics.size(); i++) {
                if (statistics(i) == "mean") {
                    textures(row + center_coord(0),
                             col + center_coord(1), i) = base_window_mean;
                } else {
                    pfunc f = stat_func_map[Rcpp::as<std::string>(statistics(i))];
                    textures(row + center_coord(0),
                             col + center_coord(1), i) = (*f)(pij, imat, jmat, mr, mc);
                }
            }

        }
    }
    return(textures);
}
