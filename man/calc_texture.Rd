\name{calc_texture}
\alias{calc_texture}
\title{Calculates a glcm texture for use in the glcm.R script}
\usage{
calc_texture(rast, n_grey, window_dims, shift, statistics, na_opt, na_val)
}
\arguments{
  \item{rast}{a matrix containing the pixels to be used in
  the texture calculation}

  \item{n_grey}{number of grey levels to use in texture
  calculation}

  \item{window_dims}{2 element list with row and column
  dimensions of the texture window}

  \item{shift}{a length 2 vector with the number of cells
  to shift when computing co-ocurrency matrices}

  \item{statistics}{a list of strings naming the texture
  statistics to calculate}

  \item{na_opt}{one of "ignore", "center", or "any"}

  \item{na_val}{what value to use to fill missing values on
  edges or where necessary due to chosen na_opt value}
}
\value{
a list of length equal to the length of the
\code{statistics} input parameter, containing the selected
textures measures
}
\description{
This function is called by the \code{\link{glcm}} function.
It is not intended to be used directly.
}
