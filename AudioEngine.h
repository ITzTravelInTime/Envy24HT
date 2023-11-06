#ifndef _Envy24HTAudioEngine_H
#define _Envy24HTAudioEngine_H

#if defined(i386) || defined(I386) || defined(IX86) || defined(__I386__) || defined(_IX86) || defined(_M_IX86) || defined(AMD64) || defined(__x86_64__) || defined(__i386__)
    #define X86
#elif defined(__PPC__) || defined(__ppc__) || defined(_ARCH_PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__powerpc__)
    #define PPC
#elif(defined(__ARM__) || defined(__arm__) || defined(_ARCH_ARM) || defined(_ARCH_ARM64) || defined(__aarch64e__) || defined(__arm) || defined(__arm64e__) || defined(__aarch64__))
    #define ARM
#else
    #error "Unknown processor architecture"
#endif

#include <IOKit/IOLib.h>
#include <IOKit/audio/IOAudioEngine.h>

#include "AudioDevice.h"
#include "misc.h"

#ifndef _IOBUFFERMEMORYDESCRIPTOR_H
class IOBufferMemoryDescriptor;
#endif

struct memhandle
{
    // note: this is for 32-bit OS only
    size_t size;
    void * addr;          // virtual
    IOPhysicalAddress dma_handle; // physical
    
#if !defined(OLD_ALLOC)
    IOBufferMemoryDescriptor *desc;
#endif
};

#define allocation_mask ((0x000000007FFFFFFFULL) & (~((PAGE_SIZE) - 1)))

int pci_alloc(struct memhandle *h);
void pci_free(struct memhandle *h);

#define Envy24HTAudioEngine com_Envy24HTAudioEngine

class IOFilterInterruptEventSource;
class IOInterruptEventSource;

class Envy24HTAudioEngine : public IOAudioEngine
{
    OSDeclareDefaultStructors(Envy24HTAudioEngine)
    
public:

    virtual bool	init(struct CardData* i_card);
    virtual void	free();
    
    virtual bool	initHardware(IOService *provider);
    virtual void	stop(IOService *provider);
    
    virtual OSString* getGlobalUniqueID();
	
    virtual void	dumpRegisters();

	virtual IOAudioStream *createNewAudioStream(IOAudioStreamDirection direction, void *sampleBuffer, UInt32 sampleBufferSize, UInt32 channel, UInt32 channels);

    virtual IOReturn performAudioEngineStart();
    virtual IOReturn performAudioEngineStop();
    
    virtual UInt32 getCurrentSampleFrame();
    
    virtual IOReturn performFormatChange(IOAudioStream *audioStream, const IOAudioStreamFormat *newFormat, const IOAudioSampleRate *newSampleRate);

    virtual IOReturn clipOutputSamples(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, IOAudioStream *audioStream);
    virtual IOReturn convertInputSamples(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames, const IOAudioStreamFormat *streamFormat, IOAudioStream *audioStream);
    
    virtual void filterInterrupt(int index);
	
	virtual IOReturn eraseOutputSamples(const void *mixBuf,
										void *sampleBuf,
									    UInt32 firstSampleFrame,
									    UInt32 numSampleFrames,
									    const IOAudioStreamFormat *streamFormat,
										IOAudioStream *audioStream);
	
#if defined(ARM)
    virtual bool driverDesiresHiResSampleIntervals(void);
#endif
    
private:
	struct CardData				   *card;
	UInt32							currentSampleRate;
    
	//SInt32							*inputBuffer;
    //SInt32							*outputBuffer;
	//SInt32							*outputBufferSPDIF;
    
	//IOPhysicalAddress               physicalAddressInput;
	//IOPhysicalAddress               physicalAddressOutput;
	//IOPhysicalAddress               physicalAddressOutputSPDIF;
    
    struct memhandle inBuffer;
    struct memhandle outBuffer;
    struct memhandle outSPDFBuffer;
    
    static const int numberConcurentDMABuffers = 3;
    struct memhandle concurrentDMABuffers[numberConcurentDMABuffers];
    
    IOFilterInterruptEventSource *interruptEventSource;
	
	UInt32 lookUpFrequencyBits(UInt32 Frequency, const UInt32* FreqList, const UInt32* FreqBitList, UInt32 ListSize, UInt32 Default);
    
    static void interruptHandler(OSObject *owner, IOInterruptEventSource *source, int count);
    static bool interruptFilter(OSObject *owner, IOFilterInterruptEventSource *source);
};

#endif /* _Envy24HTAudioEngine_H */
