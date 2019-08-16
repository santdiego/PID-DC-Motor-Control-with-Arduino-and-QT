//    Distributed under MIT License.
//    Author: Dr. Ing. Diego D. Santiago
//    Institution: INAUT - UNSJ - CONICET
//    Email: dsantiago@inaut.unsj.edu.ar
//    Based in Qt. Terminal example

#include "serialframedecoder.h"
SerialFrameDecoder::SerialFrameDecoder()
{
  header[0] = 0xFF;
  header[1] = 0xFF;
  header[2] = 0xFD;
  header[3] = 0x00;
  new_data = false;
  clean();
}

/**
  Header1 Header2 Header3	Reserved	Packet ID	  Length1     Length2     Instruction     Param       Param	    Param         CRC1	  CRC2
  0xFF    0xFF    0xFD	0x00        ID          Len_L       Len_H       Instruction     Param 1     …         Param N       CRC_L	  CRC_H
*/

Error_Number SerialFrameDecoder::appendFrameByte(const byte& frame_byte) {
  if (rcv_idx == 0)
    cleanBuffer();
  if (rcv_idx < 4) { //header
    if (frame_byte == header[rcv_idx]) {
      tempBuffer[rcv_idx] = frame_byte;
      rcv_idx++;
      return Incomplete_Frame;
    } else {
      cleanBuffer();
      return Result_Fail;
    }
  }
  tempBuffer[rcv_idx] = frame_byte;//receive all data until length
  if (rcv_idx == 6)//Store length
  {
    size2receive.word[0] = tempBuffer[5];
    size2receive.word[1] = tempBuffer[6];
    if (size2receive.value > BUFFERSIZE - 7) {
      cleanBuffer();
      return Data_Length_Error;
    }
  }
  //the minimun full frame length is 9.
  if ( (rcv_idx >= 9) && (rcv_idx == (6 + size2receive.value))) { //All received
    byte16 crc_actual;
    crc_actual.word[0] = tempBuffer[rcv_idx - 1];
    crc_actual.word[1] = tempBuffer[rcv_idx];
    uint16_t crc_teoric = calculateCRC16(tempBuffer, rcv_idx - 1); //without CRC
    if (crc_actual.value == crc_teoric) {
      setFrame(tempBuffer, rcv_idx + 1);
      cleanBuffer();
      new_data = true;
      return No_Error;
    } else {
      cleanBuffer();
      return CRC_Error;
    }
  }
  rcv_idx++;
  return Incomplete_Frame;
}


void SerialFrameDecoder::getLastFrame(byte *frame, uint16_t &frame_sz) {
  memcpy(frame, outputBuffer, output_buffer_sz);
  frame_sz = output_buffer_sz;
}

void SerialFrameDecoder::getTempBuffer(byte *frame) {
  memcpy(frame, tempBuffer, BUFFERSIZE);
}

Error_Number SerialFrameDecoder::getInstructionPacket(byte& packet_ID, Instruction &instruction, byte *param, uint16_t& param_length) {
  new_data = false;
  if (output_buffer_sz < 10)
    return Result_Fail;
  //CHECK CRC FIRST!
  //return CRC_ERROR
  byte16 length;
  packet_ID       = outputBuffer[4];
  length.word[0]  = outputBuffer[5];
  length.word[1]  = outputBuffer[6];
  instruction     = Instruction(outputBuffer[7]);
  param_length    = length.value - 3; //remove INSTRUCTION & CRC
  memcpy(param, outputBuffer + 8, param_length);
  
  return No_Error;

}
Error_Number SerialFrameDecoder::getStatusPacket(byte& packet_ID, Instruction &instruction, Error_Number &error, byte *param, uint16_t& param_length) {
  new_data = false;
  if (output_buffer_sz < 11)
    return Result_Fail;
  //CHECK CRC FIRST!
  //return CRC_ERROR
  byte16 length;
  packet_ID       = outputBuffer[4];
  length.word[0]  = outputBuffer[5];
  length.word[1]  = outputBuffer[6];
  instruction     = Instruction(outputBuffer[7]);
  error           = Error_Number(outputBuffer[8]);
  param_length    = length.value - 4; //remove INSTRUCTION, ERROR & CRC
  memcpy(param, outputBuffer + 9, param_length);
  return No_Error;
}

void SerialFrameDecoder::setFrame(const byte *frame, const uint16_t& frame_sz) {
  if (frame_sz <= BUFFERSIZE)
    memcpy(outputBuffer, frame, frame_sz);
  output_buffer_sz = frame_sz;
}

/*Status Packet
  Header1     Header2     Header3     Reserved	  Packet ID	  Length1     Length2     Instruction     ERR     PARAM       PARAM	    PARAM       CRC1	  CRC2
  0xFF        0xFF        0xFD        0x00        ID          Len_L       Len_H       Instruction     Error	  Param 1     …        Param N     CRC_L	  CRC_H
*/
void SerialFrameDecoder::buildStatusPacket(byte dev_id, const byte *data, uint16_t data_size, byte error) {
  clean();
  //if (input_sz + 11 > BUFFERSIZE)
  //   return;
  for (size_t i = 0; i < 4; i++)
    outputBuffer[i] = header[i];

  outputBuffer[4] = dev_id;
  byte16 length;
  length.value = data_size + 4;
  outputBuffer[5] = length.word[0];
  outputBuffer[6] = length.word[1];
  outputBuffer[7] = 0x55; //Instruction
  outputBuffer[8] = error;
  for (size_t i = 0; i < data_size; i++) {
    outputBuffer[9 + i] = data[i];
  }
  //calc CRC
  byte16 crc;
  crc.value = calculateCRC16(outputBuffer, 9 + data_size);
  outputBuffer[9 + data_size] = crc.word[0]; //CRCL
  outputBuffer[10 + data_size] = crc.word[1]; //CRCH
  output_buffer_sz = 10 + data_size + 1;

}
void SerialFrameDecoder::buildInstructionPacket(byte dev_id, Instruction inst, const byte *data, uint16_t data_size) {
  clean();
  //if (input_sz + 11 > BUFFERSIZE)
  //   return;
  for (size_t i = 0; i < 4; i++)
    outputBuffer[i] = header[i];

  outputBuffer[4] = dev_id;
  byte16 length;
  length.value = data_size + 3;
  outputBuffer[5] = length.word[0];
  outputBuffer[6] = length.word[1];
  outputBuffer[7] = inst; //Instruction
  for (size_t i = 0; i < data_size; i++) {
    outputBuffer[8 + i] = data[i];
  }
  //calc CRC
  byte16 crc;
  crc.value = calculateCRC16(outputBuffer, 8 + data_size);
  outputBuffer[8 + data_size] = crc.word[0]; //CRCL
  outputBuffer[9 + data_size] = crc.word[1]; //CRCH
  output_buffer_sz = 9 + data_size + 1;

}

void SerialFrameDecoder::clean() {
  output_buffer_sz = 0;
  size2receive.value = 0;
  rcv_idx = 0;

  //For output or build frames.
  for (int i = 0; i < BUFFERSIZE; i++)
  {
    outputBuffer[i] = 0; //buffer to send data
    tempBuffer[i] = 0;
  }
}

void SerialFrameDecoder::cleanBuffer() {
  size2receive.value = 0;
  for (int i = 0; i < BUFFERSIZE; i++)
  {
    tempBuffer[i] = 0; //buffer to send data
  }
  rcv_idx = 0;
}

unsigned short SerialFrameDecoder::calculateCRC16(unsigned char *data_blk_ptr, unsigned short data_blk_size)
{
  unsigned short crc_accum = 0;
  unsigned short i, j;
  unsigned short crc_table[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
  };

  for (j = 0; j < data_blk_size; j++)
  {
    i = ((unsigned short)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
    crc_accum = (crc_accum << 8) ^ crc_table[i];
  }

  return crc_accum;
}
