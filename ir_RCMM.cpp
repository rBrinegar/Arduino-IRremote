// I used a non-traditional method for decoding the mark/space pairs

#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
//                       RRRR    CCCC  M   M  M   M
//                       R   R  C      MM MM  MM MM
//                       RRRR   C      M M M  M M M
//                       R  R   C      M   M  M   M
//                       R   R   CCCC  M   M  M   M
//
// Note : Caller needs to take care of flipping the toggle bit
//
//+=============================================================================

#define RCMM_HDR_MARK               417
#define RCMM_HDR_MARK_TICKS         RCMM_HDR_MARK / USECPERTICK
#define RCMM_HDR_SPACE              278
#define RCMM_DATA_MARK              167
#define RCMM_DATA_MARK_TICKS        RCMM_DATA_MARK / USECPERTICK
#define RCMM_ZERO_SPACE             278
#define RCMM_ONE_SPACE              444
#define RCMM_TWO_SPACE              611
#define RCMM_THREE_SPACE            778

#define RCMM_HDR_TOLERANCE           55
#define RCMM_DATA_MARK_TOLERANCE     55
#define RCMM_DATA_SPACE_TOLERANCE    83

#define RCMM_TOGGLE_32BIT        0x8000

#if SEND_RCMM
void IRsend::sendRCMM (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	// Header
	mark(RCMM_HDR_MARK);
	space(RCMM_HDR_SPACE);

    // Data
    for (int sendBits = nbits; sendBits; sendBits -= 2) {
        unsigned long sendData = ((data >> (sendBits - 2)) & 0x03UL) ;
        mark(RCMM_DATA_MARK);
        if      (sendData == 0x00) space(RCMM_ZERO_SPACE);
        else if (sendData == 0x01) space(RCMM_ONE_SPACE);
        else if (sendData == 0x02) space(RCMM_TWO_SPACE);
        else                       space(RCMM_THREE_SPACE);
    }

    // We need a trailing mark to time the last space sent
    mark(RCMM_DATA_MARK);
    space(0);  // Always end with the LED off
}
#endif

#if DECODE_RCMM
bool IRrecv::decodeRCMM(decode_results *results)
{
    unsigned long  data   = 0;
    int            offset = 1;  // Skip initial gap space
    int            val ;

    val = (results->rawbuf[offset++] + results->rawbuf[offset++]) - RCMM_HDR_MARK_TICKS ;
    if (!MATCH_WITHIN(val, RCMM_HDR_SPACE, RCMM_HDR_TOLERANCE)) return false ;

    while (offset < (results->rawlen - 1)) {
        data <<= 2 ;
        val = (results->rawbuf[offset++] + results->rawbuf[offset++]) - RCMM_DATA_MARK_TICKS ;
        if      (MATCH_WITHIN(val, RCMM_ZERO_SPACE, RCMM_DATA_SPACE_TOLERANCE)) ;
        else if (MATCH_WITHIN(val, RCMM_ONE_SPACE, RCMM_DATA_SPACE_TOLERANCE))   data += 1 ;
        else if (MATCH_WITHIN(val, RCMM_TWO_SPACE, RCMM_DATA_SPACE_TOLERANCE))   data += 2 ;
        else if (MATCH_WITHIN(val, RCMM_THREE_SPACE, RCMM_DATA_SPACE_TOLERANCE)) data += 3 ;
        else                                                                  return false ;
    }

    // Success
    results->bits        = (data & 0xff000000) ? 32 : (data & 0x00fff000) ? 24 : 12 ;
    results->value       = data ;
    results->decode_type = RCMM ;

    return true ;
}
#endif
