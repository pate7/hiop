#include "hiopMatrixSparseTriplet.hpp"

#include "blasdefs.hpp"

#include <algorithm> //for std::min
#include <cmath> //for std::isfinite
#include <cstring>

#include <cassert>

namespace hiop
{

hiopMatrixSparseTriplet::hiopMatrixSparseTriplet(int rows, int cols, int nnz_)
  : nrows(rows), ncols(cols), nnz(nnz_), row_starts(NULL)
{
  iRow = new  int[nnz];
  jCol = new int[nnz];
  values = new double[nnz];

}

hiopMatrixSparseTriplet::~hiopMatrixSparseTriplet()
{
  delete [] iRow;
  delete [] jCol;
  delete [] values;
  delete row_starts;
}

void hiopMatrixSparseTriplet::setToZero()
{
  for(int i=0; i<nnz; i++)
    values[i] = 0.;
}
void hiopMatrixSparseTriplet::setToConstant(double c)
{
  for(int i=0; i<nnz; i++)
    values[i] = c;
}

/** y = beta * y + alpha * this * x */
void hiopMatrixSparseTriplet::timesVec(double beta,  hiopVector& y,
				double alpha, const hiopVector& x ) const
{
  assert(x.get_size() == ncols);
  assert(y.get_size() == nrows);

  hiopVectorPar& yy = dynamic_cast<hiopVectorPar&>(y);
  const hiopVectorPar& xx = dynamic_cast<const hiopVectorPar&>(x);

  double* y_data = yy.local_data();
  const double* x_data = xx.local_data_const();

  timesVec(beta, y_data, alpha, x_data);
}
 
/** y = beta * y + alpha * this * x */
void hiopMatrixSparseTriplet::timesVec(double beta,  double* y,
				       double alpha, const double* x ) const
{
    // y:= beta*y
  for (int i = 0; i < nrows; i++) {
    y[i] *= beta;
  }

  // y += alpha*this*x
  for (int i = 0; i < nnz; i++) {
    assert(iRow[i] < nrows);
    assert(jCol[i] < ncols);
    y[iRow[i]] += alpha * x[jCol[i]] * values[i];
  }
}
 
/** y = beta * y + alpha * this^T * x */
void hiopMatrixSparseTriplet::transTimesVec(double beta,   hiopVector& y,
                             	     double alpha,  const hiopVector& x ) const
{
  assert(x.get_size() == nrows);
  assert(y.get_size() == ncols);

  hiopVectorPar& yy = dynamic_cast<hiopVectorPar&>(y);
  const hiopVectorPar& xx = dynamic_cast<const hiopVectorPar&>(x);
  
  double* y_data = yy.local_data();
  const double* x_data = xx.local_data_const();
  
  transTimesVec(beta, y_data, alpha, x_data);
}
 
/** y = beta * y + alpha * this^T * x */
void hiopMatrixSparseTriplet::transTimesVec(double beta,   double* y,
					    double alpha,  const double* x ) const
{
  // y:= beta*y
  for (int i = 0; i < ncols; i++) {
    y[i] *= beta;
  }
  
  // y += alpha*this^T*x
  for (int i = 0; i < nnz; i++) {
    assert(iRow[i] < nrows);
    assert(jCol[i] < ncols);
    y[jCol[i]] += alpha * x[iRow[i]] * values[i];
  }
}

void hiopMatrixSparseTriplet::timesMat(double beta, hiopMatrix& W, 
				       double alpha, const hiopMatrix& X) const
{
  assert(false && "not needed");
}

void hiopMatrixSparseTriplet::transTimesMat(double beta, hiopMatrix& W, 
					    double alpha, const hiopMatrix& X) const
{
  assert(false && "not needed");
}

void hiopMatrixSparseTriplet::timesMatTrans(double beta, hiopMatrix& W, 
					    double alpha, const hiopMatrix& X) const
{
  assert(false && "not needed");
}
void hiopMatrixSparseTriplet::addDiagonal(const double& alpha, const hiopVector& d_)
{
  assert(false && "not needed");
}
void hiopMatrixSparseTriplet::addDiagonal(const double& value)
{
  assert(false && "not needed");
}
void hiopMatrixSparseTriplet::addSubDiagonal(const double& alpha, long long start, const hiopVector& d_)
{
  assert(false && "not needed");
}

void hiopMatrixSparseTriplet::addMatrix(double alpha, const hiopMatrix& X)
{
  assert(false && "not needed");
}

/* block of W += alpha*this 
 * Note W; contains only the upper triangular entries */
void hiopMatrixSparseTriplet::addToSymDenseMatrixUpperTriangle(int row_start, int col_start, 
							       double alpha, hiopMatrixDense& W) const
{
  assert(row_start>=0 && row_start+nrows<=W.m());
  assert(col_start>=0 && col_start+ncols<=W.n());
  assert(W.n()==W.m());

  double** WM = W.get_M();
  for(int it=0; it<nnz; it++) {
    const int i = iRow[it]+row_start;
    const int j = jCol[it]+col_start;
    assert(i<W.m() && j<W.n()); assert(i>=0 && j>=0);
    assert(i<=j && "source entries need to map inside the upper triangular part of destination");
    WM[i][j] += alpha*values[it];
  }
}
/* block of W += alpha*transpose(this) 
 * Note W; contains only the upper triangular entries */
void hiopMatrixSparseTriplet::transAddToSymDenseMatrixUpperTriangle(int row_start, int col_start, 
								    double alpha, hiopMatrixDense& W) const
{
  assert(row_start>=0 && row_start+ncols<=W.m());
  assert(col_start>=0 && col_start+nrows<=W.n());
  assert(W.n()==W.m());

  double** WM = W.get_M();
  for(int it=0; it<nnz; it++) {
    const int i = jCol[it]+row_start;
    const int j = iRow[it]+col_start;
    assert(i<W.m() && j<W.n()); assert(i>=0 && j>=0);
    assert(i<=j && "source entries need to map inside the upper triangular part of destination");
    WM[i][j] += alpha*values[it];
  }
}

double hiopMatrixSparseTriplet::max_abs_value()
{
  char norm='M'; int one=1;
  double maxv = DLANGE(&norm, &one, &nnz, values, &one, NULL);
  return maxv;
}

bool hiopMatrixSparseTriplet::isfinite() const
{

#ifdef HIOP_DEEPCHECKS
  assert(this->checkIndexesAreOrdered());
#endif
  for(int i=0; i<nnz; i++)
    if(false==std::isfinite(values[i])) return false;
  return true;
}

hiopMatrix* hiopMatrixSparseTriplet::alloc_clone() const
{
  return new hiopMatrixSparseTriplet(nrows, ncols, nnz);
}

hiopMatrix* hiopMatrixSparseTriplet::new_copy() const
{
#ifdef HIOP_DEEPCHECKS
  assert(this->checkIndexesAreOrdered());
#endif
  hiopMatrixSparseTriplet* copy = new hiopMatrixSparseTriplet(nrows, ncols, nnz);
  memcpy(copy->iRow, iRow, nnz*sizeof(int));
  memcpy(copy->jCol, jCol, nnz*sizeof(int));
  memcpy(copy->values, values, nnz*sizeof(double));
  return copy;
}
void hiopMatrixSparseTriplet::copyFrom(const hiopMatrixSparseTriplet& dm)
{
  assert(false && "this is to be implemented - method def too vague for now");
}

#ifdef HIOP_DEEPCHECKS
bool hiopMatrixSparseTriplet::checkIndexesAreOrdered() const
{
  if(nnz==0) return true;
  for(int i=1; i<nnz; i++) {
    if(iRow[i] < iRow[i-1]) return false;
    /* else */
    if(iRow[i] == iRow[i-1])
      if(jCol[i] < jCol[i-1]) return false;
  }
  return true;
}
#endif

void hiopMatrixSparseTriplet::
addMatTimesDinvTimesMatTransToDiagBlockOfSymDenseMatrixUpperTriangle(int rowAndCol_dest_start,
							  const double& alpha, 
							  const hiopVectorPar& D, hiopMatrixDense& W) const
{
  const int row_dest_start = rowAndCol_dest_start, col_dest_start = rowAndCol_dest_start;
  int n = this->nrows;
  assert(row_dest_start>=0 && row_dest_start+n<=W.m());
  assert(col_dest_start>=0 && col_dest_start+nrows<=W.n());
  assert(D.get_size() == this->ncols);
  double** WM = W.get_M();
  const double* DM = D.local_data_const();

  if(row_starts==NULL) row_starts = allocAndBuildRowStarts();
  assert(row_starts);

  double acc;

  for(int i=0; i<this->nrows; i++) {
    //j==i
    acc = 0.;
    for(int k=row_starts->idx_start[i]; k<row_starts->idx_start[i+1]; k++)
      acc += this->values[k] / DM[this->jCol[k]] * this->values[k];
    WM[i+row_dest_start][i+col_dest_start] += alpha*acc;

    //j>i
    for(int j=i+1; j<this->nrows; j++) {
      //dest[i,j] = weigthed_dotprod(this_row_i,this_row_j)
      acc = 0.;

      int ki=row_starts->idx_start[i], kj=row_starts->idx_start[j];
      while(ki<row_starts->idx_start[i+1] && kj<row_starts->idx_start[j+1]) {
	assert(ki<this->nnz); 
	assert(kj<this->nnz);
	if(this->jCol[ki] == this->jCol[kj]) { 
	  acc += this->values[ki] / DM[this->jCol[ki]] * this->values[kj];
	  ki++;
	  kj++;
	} else {
	  if(this->jCol[ki]<this->jCol[kj]) ki++;
	  else                              kj++;
	}
      } //end of loop over ki and kj

      WM[i+row_dest_start][j+col_dest_start] += alpha*acc;

    } //end j
  } // end i

}

// //assumes triplets are ordered
hiopMatrixSparseTriplet::RowStartsInfo* 
hiopMatrixSparseTriplet::allocAndBuildRowStarts() const
{
  RowStartsInfo* rsi = new RowStartsInfo(nrows); assert(rsi);
  int it_triplet=0;
  rsi->idx_start[0]=0;
  for(int i=1; i<=this->nrows; i++) {
    
    rsi->idx_start[i]=rsi->idx_start[i-1];
    
    while(this->iRow[it_triplet] == i-1) {
#ifdef HIOP_DEEPCHECKS
      if(it_triplet>=1) {
	assert(iRow[it_triplet-1]<=iRow[it_triplet] && "row indexes are not sorted");
	//assert(iCol[it_triplet-1]<=iCol[it_triplet]);
	if(iRow[it_triplet-1]==iRow[it_triplet])
	  assert(jCol[it_triplet-1] < jCol[it_triplet] && "col indexes are not sorted");
      }
#endif
      rsi->idx_start[i]++;
      it_triplet++;
    }
    assert(rsi->idx_start[i] == it_triplet);
  }
  assert(it_triplet==this->nnz);
  return rsi;
}

void hiopMatrixSparseTriplet::print(FILE* file, const char* msg/*=NULL*/, 
				    int maxRows/*=-1*/, int maxCols/*=-1*/, 
				    int rank/*=-1*/) const 
{
  int myrank=0, numranks=1; //this is a local object => always print

    int max_elems = maxRows>=0 ? maxRows : nnz;
    max_elems = std::min(max_elems, nnz);

  if(myrank==rank || rank==-1) {

    if(NULL==msg) {
      if(numranks>1)
        fprintf(file, "matrix of size %lld %lld and nonzeros %lld, printing %d elems (on rank=%d)\n", 
		nrows, ncols, nnz, max_elems, myrank);
      else
        fprintf(file, "matrix of size %lld %lld and nonzeros %lld, printing %d elems\n", 
		nrows, ncols, nnz, max_elems);
    } else {
      fprintf(file, "%s ", msg);
    }    



    // using matlab indices
    fprintf(file, "iRow=[");
    for(int it=0; it<max_elems; it++)  fprintf(file, "%d ; ", iRow[it]+1);
    fprintf(file, "];\n");
    
    fprintf(file, "jCol=[");
    for(int it=0; it<max_elems; it++)  fprintf(file, "%d ; ", jCol[it]+1);
    fprintf(file, "];\n");
    
    fprintf(file, "v=[");
    for(int it=0; it<max_elems; it++)  fprintf(file, "%22.16e ; ", values[it]);
    fprintf(file, "];\n");
  }
}

/**********************************************************************************
  * Sparse symmetric matrix in triplet format. Only the lower triangle is stored
  *********************************************************************************
*/
void hiopMatrixSymSparseTriplet::timesVec(double beta,  hiopVector& y,
					  double alpha, const hiopVector& x ) const
{
  assert(ncols == nrows);
  assert(x.get_size() == ncols);
  assert(y.get_size() == nrows);

  hiopVectorPar& yy = dynamic_cast<hiopVectorPar&>(y);
  const hiopVectorPar& xx = dynamic_cast<const hiopVectorPar&>(x);

  double* y_data = yy.local_data();
  const double* x_data = xx.local_data_const();

  timesVec(beta, y_data, alpha, x_data);
}
 
/** y = beta * y + alpha * this * x */
void hiopMatrixSymSparseTriplet::timesVec(double beta,  double* y,
					  double alpha, const double* x ) const
{
  assert(ncols == nrows);
  // y:= beta*y
  for (int i = 0; i < nrows; i++) {
    y[i] *= beta;
  }

  // y += alpha*this*x
  for (int i = 0; i < nnz; i++) {
    assert(iRow[i] < nrows);
    assert(jCol[i] < ncols);
    y[iRow[i]] += alpha * x[jCol[i]] * values[i];
    if(iRow[i]!=jCol[i])
      y[jCol[i]] += alpha * x[iRow[i]] * values[i];
  }
}

hiopMatrix* hiopMatrixSymSparseTriplet::alloc_clone() const
{
  assert(nrows == ncols);
  return new hiopMatrixSymSparseTriplet(nrows, nnz);
}
hiopMatrix* hiopMatrixSymSparseTriplet::new_copy() const
{
  assert(nrows == ncols);
  hiopMatrixSymSparseTriplet* copy = new hiopMatrixSymSparseTriplet(nrows, nnz);
  memcpy(copy->iRow, iRow, nnz*sizeof(int));
  memcpy(copy->jCol, jCol, nnz*sizeof(int));
  memcpy(copy->values, values, nnz*sizeof(double));
  return copy;
}

/* block of W += alpha*this 
 * Note W; contains only the upper triangular entries */
void hiopMatrixSymSparseTriplet::addToSymDenseMatrixUpperTriangle(int row_start, int col_start, 
						  double alpha, hiopMatrixDense& W) const
{
  assert(row_start>=0 && row_start+nrows<=W.m());
  assert(col_start>=0 && col_start+ncols<=W.n());
  assert(W.n()==W.m());

  double** WM = W.get_M();
  for(int it=0; it<nnz; it++) {
    assert(iRow[it]<=jCol[it] && "sparse symmetric matrices should contain only upper triangular entries");
    const int i = iRow[it]+row_start;
    const int j = jCol[it]+col_start;
    assert(i<W.m() && j<W.n()); assert(i>=0 && j>=0);
    assert(i<=j && "symMatrices not aligned; source entries need to map inside the upper triangular part of destination");
    WM[i][j] += alpha*values[it];
  }
}
void hiopMatrixSymSparseTriplet::transAddToSymDenseMatrixUpperTriangle(int row_start, int col_start, 
								       double alpha, hiopMatrixDense& W) const
{
  assert(row_start>=0 && row_start+ncols<=W.m());
  assert(col_start>=0 && col_start+nrows<=W.n());
  assert(W.n()==W.m());

  double** WM = W.get_M();
  for(int it=0; it<nnz; it++) {
    assert(iRow[it]<=jCol[it] && "sparse symmetric matrices should contain only upper triangle entries");
    const int i = jCol[it]+row_start;
    const int j = iRow[it]+col_start;
    assert(i<W.m() && j<W.n()); assert(i>=0 && j>=0);
    assert(i<=j && "symMatrices not aligned; source entries need to map inside the upper triangular part of destination");
    WM[i][j] += alpha*values[it];
  }
}

/* extract subdiagonal from 'this' (source) and adds the entries to 'vec_dest' starting at
 * index 'vec_start'. If num_elems>=0, 'num_elems' are copied; otherwise copies as many as
 * are available in 'vec_dest' starting at 'vec_start'
 */
void hiopMatrixSymSparseTriplet::
startingAtAddSubDiagonalToStartingAt(int diag_src_start, const double& alpha, 
				     hiopVector& vec_dest, int vec_start, int num_elems/*=-1*/) const
{
  hiopVectorPar& vd = dynamic_cast<hiopVectorPar&>(vec_dest);
  if(num_elems<0) num_elems = vd.get_size();
  assert(num_elems<=vd.get_size());

  assert(diag_src_start>=0 && diag_src_start+num_elems<=this->nrows);
  double* v = vd.local_data();

  for(int itnz=0; itnz<nnz; itnz++) {
    const int row = iRow[itnz];
    if(row==jCol[itnz]) {
      if(row>=diag_src_start && row<diag_src_start+num_elems) {
	assert(row+vec_start<vd.get_size());
	v[vec_start+row] += alpha * this->values[itnz];
      }
    }
  }
}

} //end of namespace
