#ifndef __PLOTSREGISTER_HPP_
#define __PLOTSREGISTER_HPP_

#include <vector>

#include "AliasedPair.hpp"

/** Holds ranges and offsets of all plots. Singleton class. */
class PlotsRegister {
public:
    static PlotsRegister* R(void);
    
    bool CheckRange (int offset, int range) const;
    bool Add(int offset, int range);

    private:
    DEFINE_ALIASED_PAIR(MinMax, int, int, min, max);

    /** Vector of min, max of histogram numbers */
    std::vector< MinMax > reg;
    
    /** Singleton synax stuff */
    static PlotsRegister plotsReg;
    /** Dummy variable, because it doesn't work with empty constr (?). */
    PlotsRegister(bool create) {}
    /** Private copy-contructor */
    PlotsRegister(const PlotsRegister&);
};

#endif // __PLOTSREGISTER_HPP_