#include "config.h"
#include "Application.h"
#include "Bit_Stuffing_Reading.h"
#include "Decoder.h"
#include "CRC_Calculator.h"
#include "Frame_Mounter.h"
#include "Frame_Transmitter.h"
#include "Bit_Stuffing_Writing.h"
#include "Error.h"
#include "Simulator.h"
#include "BTL.h"
#include "utils.h"
#include "datatypes/datatypes.h"

// sequência de bits do frame de input
bool input[]      = {LOW , HIGH, LOW , HIGH, LOW , HIGH, LOW };
// sequência de bits do frame de input
bool output[]     = {LOW , HIGH, LOW , LOW , HIGH, HIGH, LOW };

// posição do bit segmentado em que ocorre o respectivo bit de input
uint8_t seg_pos[] = {  0 ,  0  ,  14  ,  0  ,  15  ,  0  ,  1  };
/*****************************************************************************
| -------------------------------- BIT TIME -------------------------------- |
| SYNC_SEG |             TSEG1             |              TSEG2              |
|     0    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|==========|===============================|=================================|
|     0    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |-7 | -6 | -5 | -4 | -3 | -2 | -1 |
*****************************************************************************/

String FRAME_INPUT = "10000010000011111000001000001000001000001001111101111101111101111101111101111101111101111101111101111101111101111101111101110101100001011111011111111";
//"00000000001111000000000000000000000111111111111111111111111111111111111111111111111111111111111111111110101100001011111011111111"
//"00000000011110000000000000000000"
//"00000000010000000001100000000011110000000000000000000"

//0000010000011111000001000001000001000001001111101111101111101111101111101111101111101111101111101111101111101111101111101110101100001011111011111111
//0000010000011111000001000001000001000001001111101111101111101111101111101111101111101111101111101111101111101111101111101110101100001011111011111111111

uint8_t dlc_simulation = 7;
bool ide_simulation = DOMINANT;
uint8_t max_stuffing_index;

BTL_Data btl_output;
Application_Data application_output;
CRC_Data frame_mouter_crc_interface;
CRC_Data decoder_crc_interface;
Bit_Stuffing_Reading_Data bit_stuffing_rd_output;
Bit_Stuffing_Writing_Data bit_stuffing_wr_output;
Decoder_Data decoder_output;
Frame_Mounter_Data frame_mounter_output;
Frame_Transmitter_Data frame_transmitter_output;
Error_Data error_output;

BitTimingLogic BTL;

Bit_Stuffing_Reading bit_stuffing_rd(bit_stuffing_rd_output);
Bit_Stuffing_Writing bit_stuffing_wr(bit_stuffing_wr_output);
Decoder decoder(decoder_output, MAX_FRAME_SIZE);
Frame_Mounter frame_mounter(frame_mounter_output, MAX_FRAME_SIZE);
Frame_Transmitter frame_transmitter(frame_transmitter_output);
Error error(error_output);

void setup()
{
    Serial.begin(115200);
    /*
    pinMode(TQ_CLK,   OUTPUT);
    pinMode(HARDSYNC, OUTPUT);
    pinMode(RESYNC,   OUTPUT);
    pinMode(STATE_0,  OUTPUT);
    pinMode(STATE_1,  OUTPUT);

    #if !SIMULATION
    pinMode(INPUT_BIT, INPUT);
    pinMode(WRITE_BIT, INPUT);
    pinMode(BUS_IDLE,  INPUT);
    #endif
    
    digitalWrite(TQ_CLK,   LOW);
    digitalWrite(HARDSYNC, LOW);
    digitalWrite(RESYNC,   LOW);
    digitalWrite(STATE_0,  LOW);
    digitalWrite(STATE_1,  LOW);

    BTL.setup(TQ, T1, T2, SJW);
    */
   frame_mounter.connect_inputs(application_output, frame_mouter_crc_interface);
   frame_transmitter.connect_inputs(frame_mounter_output, bit_stuffing_wr_output, bit_stuffing_rd_output, error_output, decoder_output);
   bit_stuffing_rd.connect_inputs(decoder_output, btl_output);
   bit_stuffing_wr.connect_inputs(frame_transmitter_output, btl_output);
   decoder.connect_inputs(bit_stuffing_rd_output, decoder_crc_interface, frame_transmitter_output);
   
   error_output.error_detected = LOW;

   if (ide_simulation == DOMINANT) {
       max_stuffing_index = 1 + 11 + 1 + 1 + 1 + 4 + 8*dlc_simulation + 15 - 1;
   }
   else {
       max_stuffing_index = 1 + 11 + 1 + 1 + 18 + 1 + 2 + 4 + 8*dlc_simulation + 15 - 1;
   }
}

void loop()
{
    static bool new_frame = LOW;
    static bool flag_custom_frame = true;
    static bool ACK_slot;
    static CRC_data crc_data;
    static bool FRAME[MAX_FRAME_SIZE+CRC_SIZE+1];
    static uint8_t btl_counter = 0;
    static bool flag_frame_ready_logging = true;
    static uint8_t writing_point_counter = 0;
    static uint8_t sample_point_counter = 0;

    btl_counter = btl_counter % (T1 - T2 + 1);

    if (flag_custom_frame) {
        custom_frame(application_output);
        flag_custom_frame = false;
    }
    
    if (btl_counter == 0) {
        btl_output.writing_point = HIGH;
        writing_point_counter++;
        //Serial.println("\nWRITING_POINT\n");
    }
    else if (btl_counter == T1) {
        //btl_output.sampled_bit = bit_stuffing_wr_output.output_bit;
        btl_output.sampled_bit = RECESSIVE /*(FRAME_INPUT[sample_point_counter] > '0')*/ & bit_stuffing_wr_output.output_bit;
        btl_output.sample_point = HIGH;

        sample_point_counter++;
        //Serial.println("\nSAMPLE_POINT\n");
    }
    
    bit_stuffing_rd.run();
    frame_mounter.run();
    calculate_CRC(frame_mouter_crc_interface, frame_mounter_output.FRAME);
    // FRAME = 0100100011010000001000000011101101001010001111111111
    if (frame_mounter_output.frame_ready) {
        if (flag_frame_ready_logging) {
            flag_frame_ready_logging = false;
            Serial.println("\n[FRAME MOUNTER] frame_ready");
            Serial.print("[FRAME MOUNTER] FRAME = ");

            for (int i = 0; i < frame_mouter_crc_interface.PT_COUNTER + CRC_SIZE + 3 + EOF_SIZE; i++)
            {
                Serial.print(frame_mounter_output.FRAME[i], DEC);
            }
            Serial.println();
            delay(5000);
        }
    }
    //error.run();
    
    frame_transmitter.run();
    bit_stuffing_wr.run();
    //decoder.run();
    calculate_CRC(decoder_crc_interface, decoder_output.PARTIAL_FRAME);
    
    if ((frame_mounter_output.frame_ready) && (btl_output.writing_point))  {
        Serial.print(bit_stuffing_wr_output.output_bit, DEC);
    }
    
    /*
    if (bit_stuffing_rd_output.new_sample_pt) {
        Serial.print(bit_stuffing_rd_output.new_sampled_bit);

        if (sample_point_counter >= max_stuffing_index) {
            decoder_output.stuffing_enable = LOW;
        }        
    }
    */
    /*
    if (writing_point_counter > frame_mouter_crc_interface.PT_COUNTER) {
        delay(5000);
    }
    */
    /*
    if (bit_stuffing_rd_output.new_sample_pt) {
        decoder.print_frame();
    }
    */
    btl_counter++;
    btl_output.sample_point = LOW;
    btl_output.writing_point = LOW;

    if (frame_mounter_output.frame_ready && frame_transmitter_output.EoF && writing_point_counter > frame_mouter_crc_interface.PT_COUNTER) {
        writing_point_counter = 0;
        while(true);
    }

    /*
    if (sample_point_counter == FRAME_INPUT.length()) {
        sample_point_counter = 0;
        decoder.print_frame();
        while(true);
    }
    */
    
    /*
    Serial.print("FRAME = ");
    for (int i = 0; i < MAX_FRAME_SIZE; i++)
    {
        Serial.print(frame_mounter_output.FRAME[i], DEC);
    }
    Serial.println();
    */
    /*   
    static uint8_t i = 0, j = 0;
    static bool bus_idle_BPL = HIGH;
    
    #if SIMULATION
    static bool input_bit = input[i];
    static bool write_bit = output[i];
    static bool bus_idle = HIGH;
    #else
    bool input_bit = digitalRead(INPUT_BIT);
    bool write_bit = digitalRead(WRITE_BIT);
    bool bus_idle  = digitalRead(BUS_IDLE);
    #endif

    static bool output_bit = LOW;
    static bool sampled_bit = LOW;
    
    static bool sample_point = LOW;
    static bool writing_point = LOW;
    bool tq = false;
    
    if (i < sizeof(seg_pos)) {
        if (BTL.nextTQ(seg_pos[i], j, tq)) {
            #if SIMULATION
            i++;
            input_bit = input[i];
            write_bit = output[i];
            #endif
        }

        #if SERIAL_PLOT & !LOGGING
        
        Serial.print(digitalRead(TQ_CLK), DEC);
        Serial.print(" ");
        Serial.print(input_bit+2, DEC);
        Serial.print(" ");
        Serial.print(bus_idle+4, DEC);
        Serial.print(" ");
        Serial.print(digitalRead(HARDSYNC)+6, DEC);
        Serial.print(" ");
        Serial.print(digitalRead(RESYNC)+8, DEC);
        Serial.print(" ");
        Serial.print(digitalRead(STATE_0)+10, DEC);
        Serial.print(" ");
        Serial.print(digitalRead(STATE_1)+10, DEC);
        Serial.print(" ");
        Serial.print(sample_point+12, DEC);
        Serial.print(" ");
        Serial.print(writing_point+14, DEC);
        Serial.print(" ");
        Serial.println(16, DEC);
        Serial.print(" ");
        
        #endif
        
        BTL.run(tq, input_bit, write_bit, sampled_bit, output_bit, bus_idle, sample_point, writing_point);
    }
    */
}
