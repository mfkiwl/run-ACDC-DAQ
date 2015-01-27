///////////////////////////////////////////////////////////////////
/*
definitions and prototypes for LAPPD PSEC4 DAQ readout
updated 31-10-2014
author: eric oberla
*/
///////////////////////////////////////////////////////////////////

#ifndef __SUMO_H__
#define __SUMO_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "stdUSB.h"
#include "Definitions.h"
#include "Packet.h"

enum cc_readout_mode { USB = 0, USB2x = 1, ETH = 2, UNK = 3};

class SuMo{

 public:
  SuMo();
  ~SuMo();
  
  void sync_usb(bool SYNC);

  //instructions to hardware 
  void reset_dll();
  void reset_self_trigger(unsigned int mask, int device = 0);
  void reset_time_stamp();
  void reset_acdc();
  void align_lvds();
  //front-end card specific instructions
  void toggle_LED(bool EN);
  void toggle_CAL(bool EN, int device = 0); 
  
  void set_self_trigger(bool ENABLE_TRIG, bool SYS_TRIG_OPTION, 
			bool RATE_ONLY, bool TRIG_SIGN, bool USE_SMA, 
			unsigned int mask, int device = 0);
  void set_self_trigger_mask(int mask, bool HILO, unsigned int board_mask, int device = 0);
  //chip-level instructions
  void set_pedestal_value(unsigned int PED_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);
  void set_dll_vdd(unsigned int VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);
  void set_trig_threshold(unsigned int TRIG_VALUE, unsigned int mask, int device = 0, unsigned int psec_mask = 31);
  void set_ro_target_count(unsigned int TARGET_RO_COUNT, unsigned int mask, int device = 0, unsigned int psec_mask = 31);
 
  //manage PC interface
  void set_usb_read_mode(unsigned int READ_MODE);
  void set_usb_read_mode_slaveDevice(unsigned int READ_MODE);
  void manage_cc_fifo(bool VALUE);
  void manage_cc_fifo_slaveDevice(bool VALUE);

  void hard_reset();

  //send trigger over software
  void software_trigger(unsigned int SOFT_TRIG_MASK);
  void software_trigger_slaveDevice(unsigned int SOFT_TRIG_MASK);
  
  //readout functions
  int  check_readout_mode();
  int  read_CC(bool SHOW_CC_STATUS, bool SHOW_AC_STATUS, int device = 0);
  /* this function modifies class variables BOARDS_READOUT & BOARDS_TIMEOUT for additional retval handling: */
  int  read_AC(unsigned int trig_mode, bool* mask, bool FILESAVE);  

  void dump_data();
  
  int  get_AC_info(bool PRINT, int AC_adr);             //parse meta-data from raw data stream
  int  generate_ped(bool ENABLE_FILESAVE);              //generate pedestal calibration files for each active board (one per sample cell)
  int  make_count_to_voltage(void);                     //make count-to-voltage LUT for each active board (# active boards * 6 channels * 1536 cells * 4096 !!)
  int  make_count_to_voltage(bool COPY, bool* range);
  int  load_ped();
  int  oscilloscope(int trig_mode, int numFrames, int AC_adr, int range[2]);
  int  log_data(const char* log_filename, unsigned int NUM_READS, int trig_mode, int acq_rate);
 
  int  check_active_boards(void);
  int  check_active_boards(bool print);
  int  check_active_boards(int NUM);
  int  check_active_boards_slaveDevice(void);
  
  bool DC_ACTIVE[numFrontBoards];           //TRUE if boards are connected and synced
  bool EVENT_FLAG[numFrontBoards];
  bool BOARDS_READOUT[numFrontBoards];      //TRUE if board was successfully readout when data requested
  bool BOARDS_TIMEOUT[numFrontBoards];      //TRUE if board was unsuccessfully readout when data requested (timeout error)
 
  struct packet_t *adcDat[numFrontBoards];
  enum cc_readout_mode mode;

 private:
  
  void createUSBHandles();
  void closeUSBHandles();
  int  unwrap(int ASIC);
  void unwrap_baseline(int *baseline, int ASIC);
  void form_meta_data(int Address, int count, double time, time_t now);

  struct cal_t{
    unsigned short raw_ped_data_array[AC_CHANNELS][psecSampleCells][num_ped_reads];
    /* calibration arrays */
    unsigned int           PED_DATA[AC_CHANNELS][psecSampleCells];
    float                  PED_RMS[AC_CHANNELS][psecSampleCells]; 
    bool                   PED_SUCCESS;
  } calData [numFrontBoards];

  static int compare ( const void * a, const void * b){
    return(*(unsigned short*)a - *(unsigned short*)b);
  }
   
  int put_lut_on_heap(bool* range);                    //cpu memory management when taking full count-to-voltage calibration scan
  void free_lut_from_heap(bool* range);                //cpu memory management when taking full count-to-voltage calibration scan  
  
  float***           LUT_CELL; 
  int***             LUT_CELL_COPY; 
  float***           LUT;
  float***           LUT_COPY;
  unsigned int       PED_DATA[numFrontBoards][AC_CHANNELS][psecSampleCells];
  
  /* metadata from CC */
  unsigned int       CC_INFO[cc_buffersize];  
  unsigned int       LAST_CC_INSTRUCT;
  unsigned int       CC_TIMESTAMP_LO;
  unsigned int       CC_TIMESTAMP_HI;
  unsigned int       CC_EVENT_COUNT;
  unsigned int       CC_BIN_COUNT;
  unsigned int       CC_EVENT_NO;

 protected:
  stdUSB         usb;        //object to handle USB device
  stdUSBSlave    usb2;       //object to handle USB slave device
  
};       
#endif
