#ifndef BTL_H_INCLUDE
#define BTL_H_INCLUDE

class BitTimingLogic {
    private:
        bool resync;
        bool hardsync;
        int8_t limit_TSEG1;
        int8_t limit_TSEG2;
        int8_t sjw;
        void frequency_divider(uint32_t);
        void edge_detector(bool &prev_input_bit, bool input_bit, bool &bus_idle);
        void bit_segmenter(bool &prev_input_bit, bool input_bit, bool &sample_point, bool &writing_point);

    public:
        BitTimingLogic();
        void setup(uint32_t _TQ, int8_t _T1, int8_t _T2, int8_t _SJW);
        void run(bool &tq, bool input_bit, bool write_bit, bool &sample_bit, bool &output_bit, bool &bus_idle, bool &sample_point, bool &writing_point);
        bool nextTQ(uint8_t pos, uint8_t &j, bool &tq);
};

enum {
    SYNC_SEG = 0,
    TSEG1,
    TSEG2
};

#endif
