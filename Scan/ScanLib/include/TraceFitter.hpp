#ifndef TRACE_FITTER_HPP
#define TRACE_FITTER_HPP

#include "XiaData.hpp"

/**The Paulauskas function is described in NIM A 737 (22), with a slight 
 * adaptation. We use a step function such that f(x < phase) = baseline.
 * In addition, we also we formulate gamma such that the gamma in the paper is
 * gamma_prime = 1 / pow(gamma, 0.25).
 *
 * The parameters are:
 * p[0] = baseline
 * p[1] = amplitude
 * p[2] = phase
 * p[3] = beta
 * p[4] = gamma
 *
 * \param[in] x X value.
 * \param[in] p Paramater values.
 *
 * \return the value of the function for the specified x value and parameters.
 */
double paulauskas(double *x, double *p);

class TF1;

class TraceFitter{
  protected:
	TF1 *func;
	
	size_t fittingLow;
	size_t fittingHigh;
	
	double adcTimeStep;
	double beta;
	double gamma;

  public:
	TraceFitter();

	TraceFitter(const char* func_);
		
	TraceFitter(double (*func_)(double *, double *), int npar_);

	virtual ~TraceFitter(){}

	/// Return a pointer to the TF1 parameter array.
	double *GetParameters(){ return func->GetParameters(); }

	/// Return a pointer to the root TF1 function.
	TF1 *GetFunction(){ return func; }
	
	/// Set the range of the fit as [maxIndex-low_, maxIndex+high_].
	bool SetFitRange(const size_t &low_, const size_t &high_);
	
	/// Set the fixed beta and gamma parameters.
	bool SetBetaGamma(const double &beta_, const double &gamma_);
	
	/// Fit a single trace.
	virtual bool FitPulse(ChannelEvent *event_);	
};

#endif
