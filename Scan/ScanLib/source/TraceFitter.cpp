#include "TF1.h"
#include "TGraph.h"

#include "TraceFitter.hpp"

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
double paulauskas(double *x, double *p){
	double diff = x[0] - p[2];
	if (diff < 0 ) return p[0];
	return p[0] + p[1] * std::exp(-diff * p[3]) * (1 - std::exp(-std::pow(diff * p[4],4)));
}

///////////////////////////////////////////////////////////////////////////////
// class TraceFitter
///////////////////////////////////////////////////////////////////////////////

TraceFitter::TraceFitter() : fittingLow(5), fittingHigh(10), adcTimeStep(4), beta(0.5), gamma(0.1) {
	func = new TF1("func", paulauskas, 0, 1, 5);
	func->SetParNames("baseline","amplitude","phase","beta","gamma");
}

TraceFitter::TraceFitter(const char* funcStr_) : fittingLow(5), fittingHigh(10), adcTimeStep(4), beta(0.5), gamma(0.3) {
	func = new TF1("func", funcStr_, 0, 1);
}

TraceFitter::TraceFitter(double (*funcPtr_)(double *, double *), int npar_) : fittingLow(5), fittingHigh(10), adcTimeStep(4), beta(0.5), gamma(0.3) {
	func = new TF1("func", funcPtr_, 0, 1, npar_);
}

TraceFitter::~TraceFitter(){
	delete func;
}

/// Return a pointer to the TF1 parameter array.
double *TraceFitter::GetParameters(){
	return func->GetParameters(); 
}

/// Set the range of the fit as [maxIndex-low_, maxIndex+high_].
bool TraceFitter::SetFitRange(const size_t &low_, const size_t &high_){
	if(low_ >= high_) return false;
	
	fittingLow = low_;
	fittingHigh = high_;
	
	return true;
}

/// Set the fixed beta and gamma parameters.
bool TraceFitter::SetBetaGamma(const double &beta_, const double &gamma_){
	beta = beta_;
	gamma = gamma_;

	return true;
}

bool TraceFitter::FitPulse(ChannelEvent *event_){
	if(!event_){ return false; }
	
	// "Convert" the trace into a TGraph for fitting.
	unsigned short startIndex = event_->max_index-fittingLow;
	TGraph *graph = new TGraph(fittingLow + fittingHigh);
	for(size_t graphIndex = 0; graphIndex < (fittingLow + fittingHigh); graphIndex++)
		graph->SetPoint(graphIndex, startIndex+graphIndex, event_->adcTrace[startIndex+graphIndex]);

	// Set the initial fitting conditions.
	func->FixParameter(0, event_->baseline); // Baseline of pulse
	func->SetParameter(1, event_->max_ADC/0.0247056); // Normalization of pulse
	func->SetParameter(2, event_->max_index-fittingLow); // Phase (leading edge of pulse) (adc clock ticks)
	func->FixParameter(3, beta);
	func->FixParameter(4, gamma);

	// Set the fitting range.
	func->SetRange(event_->max_index-fittingLow, event_->max_index+fittingHigh);

	// And finally, do the fitting.
	graph->Fit(func, "QR");
	
	// Update the phase of the trace.
	//event_->baseline = func->GetParameter(0);
	event_->phase = func->GetParameter(2);
	
	delete graph;
	
	return true;
}
