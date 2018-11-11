#ifndef BTL_H_INCLUDE
#define BTL_H_INCLUDE

class BitTimingLogic {
    private:
        bool resync;
        bool hardsync;
        int8_t limit_TSEG1;
        int8_t limit_TSEG2;
        int8_t sjw;
        void frequency_divider(uint32_t TQ);

    public:
        BitTimingLogic(uint32_t _TQ, int8_t _T1, int8_t _T2, int8_t _SJW);
        void edge_detector(bool scaled_clock, bool bus_idle);
        void bit_segmenter(bool &sample_point, bool &writing_point);        
};

#endif