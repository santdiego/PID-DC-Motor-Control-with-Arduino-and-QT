//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar


#ifndef SERIALFRAMEDECODER_H
#define SERIALFRAMEDECODER_H
#if defined(ARDUINO) || defined(__OPENCR__) || defined (__OPENCM904__)
#include <Arduino.h>
#else
#include <cstdint>
#include <cstring>
#endif
typedef unsigned char byte;
union byte16 {
  byte word[2];
  uint16_t value;
};
/**
  Error Number : When there has been an Error in the processing of the Instruction Packet.
  Value	Error	Description
  0x01	Result Fail	Failed to process the sent Instruction Packet
  0x02	Instruction Error	Undefined Instruction has been used
  Action has been used without Reg Write
  0x03	CRC Error	CRC of the sent Packet does not match
  0x04	Data Range Error	Data to be written in the corresponding Address is outside the range of the minimum/maximum value
  0x05	Data Length Error	Attempt to write Data that is shorter than the data length of the corresponding Address
  (ex: when you attempt to only use 2 bytes of a item that has been defined as 4 bytes)
  0x06	Data Limit Error	Data to be written in the corresponding Address is outside of the Limit value
  0x07	Access Errer	Attempt to write a value in an Address that is Read Only or has not been defined
  Attempt to read a value in an Address that is Write Only or has not been defined
  Attempt to write a value in the ROM domain while in a state of Torque Enable(ROM Lock)
*/
enum Error_Number {No_Error=0, Result_Fail = 1, Instruction_Error, CRC_Error, Data_Range_Error, Data_Length_Error, Data_Limit_Error, Access_Errer, Incomplete_Frame};

/*
 * Instruction
The field that defines the type of command.

Value Instructions  Description
0x01  Ping  Instruction that checks whether the Packet has arrived to a device with the same ID as Packet ID
0x02  Read  Instruction to read data from the Device
0x03  Write Instruction to write data on the Device
0x04  Reg Write Instruction that registers the Instruction Packet to a standby status; Packet is later executed through the Action command
0x05  Action  Instruction that executes the Packet that was registered beforehand using Reg Write
0x06  Factory Reset Instruction that resets the Control Table to its initial factory default settings
0x08  Reboot  Instruction to reboot the Device
0x10  Clear Instruction to reset certain information
0x55  Status(Return)  Return Instruction for the Instruction Packet
0x82  Sync Read For multiple devices, Instruction to read data from the same Address with the same length at once
0x83  Sync Write  For multiple devices, Instruction to write data on the same Address with the same length at once
0x92  Bulk Read For multiple devices, Instruction to read data from different Addresses with different lengths at once
0x93  Bulk Write  For multiple devices, Instruction to write data on different Addresses with different lengths at once
 */
enum Instruction{Ping=0x01,Read=0x02,Write=0x03,Reg_Write=0x04,Action=0x05,Factory_Reset=0x06,Reboot=0x08,Clear=0x10,Status=0x55,Sync_Read=0x82,Sync_Write=0x83,Bulk_Read=0x92,Bulk_Write=0x93};


/**
  Header1 Header2 Header3	Reserved	Packet ID	Length1     Length2     Instruction	Param       Param	Param       CRC1	CRC2
  0xFF    0xFF    0xFD	0x00        ID          Len_L       Len_H       Instruction	Param 1     …       Param N     CRC_L	CRC_H
*/
#define BUFFERSIZE 50
class SerialFrameDecoder
{

  private:

    byte header[4];
    //For receive new frames.
    byte tempBuffer[BUFFERSIZE]; //buffer to receive data
    byte16  size2receive; //expected size of the frame (frome instruction to CRC)
    uint16_t  rcv_idx; //index of received byte

    //For output or build frames.
    byte outputBuffer[BUFFERSIZE]; //buffer to send data
    uint16_t  output_buffer_sz;
    bool new_data;

  public:

    SerialFrameDecoder();

    void getTempBuffer(byte *frame);

    Error_Number appendFrameByte(const byte& frame_byte);

    /// \brief getLastFrame returns the last valid frame stored in the internal buffer.
    /// \param dest : pointer where data is copied. Shud have memory allocated
    /// \param frame_sz
    void getLastFrame(byte *dest, uint16_t &size);
    
    ///
    /// \brief getInstruction   Packet decodes internal buffer into instruction
    /// \param packet_ID        Id to which the package is destined
    /// \param instruction      Instruction for the device
    /// \param param            Arguments of the instruction
    /// \param param_length     Length of the arguments of the instruction
    /// \return
    Error_Number getInstructionPacket(byte& packet_ID, Instruction &instruction, byte *param, uint16_t& param_length);

    ///
    /// \brief getStatusPacket  Packet decodes internal buffer into status
    /// \param packet_ID        Id of the device that notifies its status
    /// \param instruction      If the frame is a state, then this value is always 0x55
    /// \param error            The field that indicates the processing result of Instruction Packet
    /// \param param            Pointer to the buffer where the results of the last instruction received by the device are copied.
    /// \param param_length     Length of the data pointed by param.
    /// \return
    ///
    Error_Number getStatusPacket(byte& packet_ID, Instruction &instruction,Error_Number &error, byte *param, uint16_t& param_length);

    /// \brief buildStatusPacket build a frame of status from device
    /// \param data data data from which the frame is formed
    /// \param input_sz  data size
    void buildStatusPacket(byte dev_id,const byte *data, uint16_t data_size, byte error = No_Error);
    void buildInstructionPacket(byte dev_id,Instruction inst, const byte *data, uint16_t data_size);
    ///
    /// \brief setFrame set hole frame.  v
    /// \param frame //frame to be copied to outputbuffer
    /// \param frame_sz

    void setFrame(const byte *frame, const uint16_t &frame_sz);
    ///
    /// \brief clean cleans internal states.
    ///
    void clean();

    ///
    /// \brief cleanBuffer cleans internal buffer only. Las valid data is retained.
    ///
    void cleanBuffer();

    bool newData(){return new_data;}
    unsigned short calculateCRC16(unsigned char *data_blk_ptr, unsigned short data_blk_size);
};

#endif // SERIALFRAMEDECODER_H
