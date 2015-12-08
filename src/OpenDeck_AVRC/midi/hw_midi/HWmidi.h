#ifndef LIB_MIDI_H_
#define LIB_MIDI_H_

#include <inttypes.h>
#include "..\hardware/uart/UART.h"
#include "..\Types.h"
#include "..\midi\MIDIsettings.h"

/*
    ###############################################################
    #                                                             #
    #    CONFIGURATION AREA                                       #
    #                                                             #
    #    Here are a few settings you can change to customize      #
    #    the library for your own project. You can for example    #
    #    choose to compile only parts of it so you gain flash     #
    #    space and optimize the speed of your sketch.             #
    #                                                             #
    ###############################################################
 */


#define COMPILE_MIDI_IN         1           // Set this setting to 1 to use the MIDI input.
#define COMPILE_MIDI_OUT        1           // Set this setting to 1 to use the MIDI output. 

#define USE_SERIAL_PORT         uart

#define USE_CALLBACKS           0           // Set this to 1 if you want to use callback handlers (to bind your functions to the library).
                                            // To use the callbacks, you need to have COMPILE_MIDI_IN set to 1

#define USE_1BYTE_PARSING       1           // Each call to MIDI.read will only parse one byte (might be faster).


// END OF CONFIGURATION AREA 
// (do not modify anything under this line unless you know what you are doing)


#define MIDI_CHANNEL_OMNI       0
#define MIDI_CHANNEL_OFF        17          // and over

/*! Type definition for practical use (because "unsigned char" is a bit long to write.. )*/
typedef uint8_t byte;
typedef uint16_t word;

/*! The midimsg structure contains decoded data of a MIDI message read from the serial port with read() or thru(). \n */
struct midimsg  {

    /*! The MIDI channel on which the message was received. \n Value goes from 1 to 16. */
    byte channel; 

    /*! The type of the message (see the define section for types reference) */
    midiMessageType type;

    /*! The first data byte.\n Value goes from 0 to 127.\n */
    byte data1;

    /*! The second data byte. If the message is only 2 bytes long, this one is null.\n Value goes from 0 to 127. */
    byte data2;

    /*! System Exclusive dedicated byte array. \n Array length is stocked on 16 bits, in data1 (LSB) and data2 (MSB) */
    byte sysex_array[MIDI_SYSEX_ARRAY_SIZE];

    /*! This boolean indicates if the message is valid or not. There is no channel consideration here, validity means the message respects the MIDI norm. */
    bool valid;

};


/*! \brief The main class for MIDI handling.\n
    See member descriptions to know how to use it,
    or check out the examples supplied with the library.
 */
class MIDI_Class {

public:
    //Constructor
    MIDI_Class();

    void begin(uint8_t);

/* ####### OUTPUT COMPILATION BLOCK ####### */  
#if COMPILE_MIDI_OUT

public: 

    void sendNoteOn(byte NoteNumber,byte Velocity,byte Channel);
    void sendNoteOff(byte NoteNumber,byte Velocity,byte Channel);
    void sendControlChange(byte ControlNumber, byte ControlValue,byte Channel);
    void sendProgramChange(byte ProgramNumber,byte Channel);
    void sendPitchBend(uint16_t PitchValue, byte Channel);
    void sendSysEx(int length, const byte *const array,bool ArrayContainsBoundaries = false);

    void send(midiMessageType type, byte param1, byte param2, byte channel);

private:

    const byte  genstatus(const midiMessageType inType,const byte inChannel) const;

#endif  // COMPILE_MIDI_OUT

/* ####### INPUT COMPILATION BLOCK ####### */
#if COMPILE_MIDI_IN 

public:

    bool read();
    bool read(const byte Channel);

    // Getters
    midiMessageType getType() const;
    byte getChannel() const;
    byte getData1() const;
    byte getData2() const;
    uint8_t* getSysExArray();
    int16_t getSysExArrayLength();
    bool check() const;

    byte getInputChannel() const    {

        return mInputChannel;

    }

    // Setters
    void setInputChannel(const byte Channel);

    /*! \brief Extract an enumerated MIDI type from a status byte.

     This is a utility static method, used internally, made public so you can handle kMIDITypes more easily.
     */
    static inline const midiMessageType getTypeFromStatusByte(const byte inStatus)    {

        if (
            (inStatus < 0x80)   ||
            (inStatus == 0xF4)  ||
            (inStatus == 0xF5)  ||
            (inStatus == 0xF9)  ||
            (inStatus == 0xFD)
           )    return midiMessageInvalidType;                 //data bytes and undefined

        if (inStatus < 0xF0)    return (midiMessageType)(inStatus & 0xF0);    //channel message, remove channel nibble
        else                    return (midiMessageType)(inStatus);

    }

#if USE_CALLBACKS

    void setHandleNoteOff(void (*fptr)(byte channel, byte note, byte velocity));
    void setHandleNoteOn(void (*fptr)(byte channel, byte note, byte velocity));
    void setHandleControlChange(void (*fptr)(byte channel, byte number, byte value));
    void setHandleProgramChange(void (*fptr)(byte channel, byte number));
    void setHandleHwMIDIpitchBend(void (*fptr)(byte channel, int bend));
    void setHandleSystemExclusive(void (*fptr)(byte * array, byte size));

#endif // USE_CALLBACKS

private:

    bool input_filter(byte inChannel);
    bool parse(byte inChannel);
    void reset_input_attributes();

    // Attributes
    byte            mRunningStatus_RX;
    byte            mInputChannel;

    byte            mPendingMessage[MIDI_SYSEX_ARRAY_SIZE];
    unsigned int    mPendingMessageExpectedLenght;
    unsigned int    mPendingMessageIndex;                   // Extended to unsigned int for larger sysex payloads.

    midimsg         mMessage;

#if USE_CALLBACKS

    void launchCallback();

    void (*mNoteOffCallback)(byte channel, byte note, byte velocity);
    void (*mNoteOnCallback)(byte channel, byte note, byte velocity);
    void (*mControlChangeCallback)(byte channel, byte, byte);
    void (*mProgramChangeCallback)(byte channel, byte);
    void (*mHwMIDIpitchBendCallback)(byte channel, int);
    void (*mSystemExclusiveCallback)(byte * array, byte size);

#endif // USE_CALLBACKS

#endif // COMPILE_MIDI_IN

};

extern MIDI_Class hwMIDI;

#endif