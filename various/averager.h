#ifndef	__AVERAGER__
#define	__AVERAGER__

#include	"stdint.h"

class   average {
public:
                        average         (int16_t);
                        ~average        (void);
        float           filter          (float);
        void            clear           (float);
private:
        int16_t         size;
        float           *vec;
        int16_t         filp;
};

#endif
