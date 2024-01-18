#include "mxmusicmanager.h"

#include "mxomni.h"
#include "mxticklemanager.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(MxMusicManager, 0x58);

// FUNCTION: LEGO1 0x100c05a0
MxMusicManager::MxMusicManager()
{
	Init();
}

// FUNCTION: LEGO1 0x100c0630
MxMusicManager::~MxMusicManager()
{
	Destroy(TRUE);
}

// FUNCTION: LEGO1 0x100c0690
void MxMusicManager::Init()
{
	m_multiplier = 100;
	InitData();
}

// FUNCTION: LEGO1 0x100c06a0
void MxMusicManager::InitData()
{
	m_midiStreamH = 0;
	m_midiInitialized = FALSE;
	m_bufferSize = 0;
	m_bufferCurrentSize = 0;
	m_bufferOffset = 0;
	m_bufferCurrentOffset = 0;
	m_loopCount = 0;
	m_midiHdrP = NULL;
}

// FUNCTION: LEGO1 0x100c06c0
void MxMusicManager::Destroy(MxBool p_fromDestructor)
{
	if (m_thread) {
		m_thread->Terminate();
		if (m_thread) {
			delete m_thread;
		}
	}
	else {
		TickleManager()->UnregisterClient(this);
	}

	m_criticalSection.Enter();
	DeinitializeMIDI();
	Init();
	m_criticalSection.Leave();

	if (!p_fromDestructor) {
		MxAudioManager::Destroy();
	}
}

// FUNCTION: LEGO1 0x100c0720
MxResult MxMusicManager::ResetStream()
{
	MxResult result = FAILURE;
	if (m_midiInitialized) {
		if (m_bufferCurrentSize == 0) {
			if (m_loopCount != -1) {
				m_loopCount += -1;
				if (!m_loopCount) {
					DeinitializeMIDI();
					return result;
				}
			}
			ResetBuffer();
		}
		do {
			if (m_midiHdrP->dwFlags & (MHDR_DONE | MHDR_PREPARED)) {
				if (midiOutUnprepareHeader((HMIDIOUT) m_midiStreamH, m_midiHdrP, sizeof(MIDIHDR)))
					break;
				memset(m_midiHdrP, 0, sizeof(MIDIHDR));
			}
			m_bufferCurrentOffset += 4;
			DWORD length = *((DWORD*) m_bufferCurrentOffset);
			m_bufferCurrentOffset += 4;
			m_midiHdrP->lpData = (LPSTR) m_bufferCurrentOffset;
			m_midiHdrP->dwBufferLength = length;
			m_midiHdrP->dwBytesRecorded = length;
			if (!midiOutPrepareHeader((HMIDIOUT) m_midiStreamH, m_midiHdrP, sizeof(MIDIHDR))) {
				if (!midiStreamOut(m_midiStreamH, m_midiHdrP, sizeof(MIDIHDR))) {
					result = SUCCESS;
					m_bufferCurrentOffset += length;
					m_bufferCurrentSize--;
				}
			}
		} while (FALSE);
	}
	return result;
}

// FUNCTION: LEGO1 0x100c07e0
void MxMusicManager::ResetBuffer()
{
	m_bufferCurrentOffset = m_bufferOffset;
	m_bufferCurrentSize = m_bufferSize;
}

// FUNCTION: LEGO1 0x100c07f0
void MxMusicManager::SetMIDIVolume()
{
	MxS32 result = (m_volume * m_multiplier) / 0x64;
	HMIDISTRM streamHandle = m_midiStreamH;

	if (streamHandle) {
		MxS32 volume = CalculateVolume(result);
		midiOutSetVolume((HMIDIOUT) streamHandle, volume);
	}
}

// FUNCTION: LEGO1 0x100c0820
static void CALLBACK
MidiCallbackProc(HMIDIOUT p_hmo, UINT p_wMsg, MxMusicManager* p_dwInstance, MxU32* p_dwParam1, MxU32* p_dwParam2)
{
	if (p_wMsg == MOM_DONE)
		p_dwInstance->ResetStream();
}

// FUNCTION: LEGO1 0x100c0840
MxResult MxMusicManager::Create(MxU32 p_frequencyMS, MxBool p_createThread)
{
	MxResult status = FAILURE;
	MxBool locked = FALSE;

	if (MxAudioManager::InitPresenters() == SUCCESS) {
		if (p_createThread) {
			m_criticalSection.Enter();
			locked = TRUE;
			m_thread = new MxTickleThread(this, p_frequencyMS);

			if (!m_thread || m_thread->Start(0, 0) != SUCCESS)
				goto done;
		}
		else
			TickleManager()->RegisterClient(this, p_frequencyMS);

		status = SUCCESS;
	}

done:
	if (status != SUCCESS)
		Destroy();

	if (locked)
		m_criticalSection.Leave();

	return status;
}

// FUNCTION: LEGO1 0x100c0930
void MxMusicManager::Destroy()
{
	Destroy(FALSE);
}

// FUNCTION: LEGO1 0x100c0940
void MxMusicManager::SetVolume(MxS32 p_volume)
{
	MxAudioManager::SetVolume(p_volume);
	m_criticalSection.Enter();
	SetMIDIVolume();
	m_criticalSection.Leave();
}

// FUNCTION: LEGO1 0x100c0970
void MxMusicManager::SetMultiplier(MxS32 p_multiplier)
{
	m_criticalSection.Enter();
	m_multiplier = p_multiplier;
	SetMIDIVolume();
	m_criticalSection.Leave();
}

// FUNCTION: LEGO1 0x100c09a0
MxS32 MxMusicManager::CalculateVolume(MxS32 p_volume)
{
	MxS32 result = (p_volume * 0xffff) / 100;
	return (result << 0x10) | result;
}

// FUNCTION: LEGO1 0x100c09c0
undefined4 MxMusicManager::InitializeMIDI(MxU8* p_data, MxS32 p_loopCount)
{
	MxResult result = FAILURE;
	m_criticalSection.Enter();
	do {
		if (m_midiInitialized)
			break;
		MxU32 total = midiOutGetNumDevs();
		MxU32 device = 0;
		for (; device < total; device++) {
			MIDIOUTCAPSA caps;
			midiOutGetDevCapsA(device, &caps, sizeof(MIDIOUTCAPSA));
			if (caps.wTechnology == MOD_FMSYNTH)
				break;
		}
		if (device == total)
			device = -1;
		if (midiStreamOpen(
				(LPHMIDIOUT) &m_midiStreamH,
				&device,
				1,
				(DWORD) MidiCallbackProc,
				(DWORD) this,
				CALLBACK_FUNCTION
			))
			break;
		GetMIDIVolume(m_midiVolume);
		m_midiHdrP = new MIDIHDR();
		if (!m_midiHdrP)
			break;
		memset(m_midiHdrP, 0, sizeof(MIDIHDR));
		MIDIPROPTIMEDIV timediv;
		timediv.cbStruct = 8;
		m_bufferOffset = p_data;
		m_bufferOffset += 0x14;
		timediv.dwTimeDiv = *((DWORD*) m_bufferOffset);
		if (midiStreamProperty(m_midiStreamH, (LPBYTE) &timediv, MIDIPROP_SET | MIDIPROP_TIMEDIV))
			break;
		m_bufferOffset += 0x14;
		m_bufferSize = *((MxU32*) m_bufferOffset);
		m_bufferOffset += 0x4;
		m_loopCount = p_loopCount;
		m_midiInitialized = TRUE;
		ResetBuffer();
		if (ResetStream() != SUCCESS)
			break;
		SetMIDIVolume();
		if (midiStreamRestart(m_midiStreamH))
			break;
		result = SUCCESS;
	} while (FALSE);
	m_criticalSection.Leave();
	return result;
}

// FUNCTION: LEGO1 0x100c0b20
void MxMusicManager::DeinitializeMIDI()
{
	m_criticalSection.Enter();

	if (m_midiInitialized) {
		m_midiInitialized = FALSE;
		midiStreamStop(m_midiStreamH);
		midiOutUnprepareHeader((HMIDIOUT) m_midiStreamH, m_midiHdrP, sizeof(MIDIHDR));
		midiOutSetVolume((HMIDIOUT) m_midiStreamH, m_midiVolume);
		midiStreamClose(m_midiStreamH);
		delete m_midiHdrP;
		InitData();
	}

	m_criticalSection.Leave();
}
