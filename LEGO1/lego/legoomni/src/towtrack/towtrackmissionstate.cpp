#include "towtrackmissionstate.h"

DECOMP_SIZE_ASSERT(TowTrackMissionState, 0x28)

// FUNCTION: LEGO1 0x1004dd30
TowTrackMissionState::TowTrackMissionState()
{
	m_unk0x12 = 0;
	m_unk0x14 = 0;
	m_unk0x16 = 0;
	m_unk0x08 = 0;
	m_unk0x18 = 0;
	m_unk0x0c = 0;
	m_unk0x1a = 0;
	m_unk0x10 = 0;
	m_unk0x1c = 0;
	m_color1 = 0;
	m_color2 = 0;
	m_color3 = 0;
	m_color4 = 0;
}

// FUNCTION: LEGO1 0x1004dde0
MxResult TowTrackMissionState::Serialize(LegoFile* p_legoFile)
{
	LegoState::Serialize(p_legoFile);

	if (p_legoFile->IsReadMode()) {
		p_legoFile->Read(&m_unk0x12, sizeof(m_unk0x12));
		p_legoFile->Read(&m_unk0x14, sizeof(m_unk0x14));
		p_legoFile->Read(&m_unk0x16, sizeof(m_unk0x16));
		p_legoFile->Read(&m_unk0x18, sizeof(m_unk0x18));
		p_legoFile->Read(&m_unk0x1a, sizeof(m_unk0x1a));
		p_legoFile->Read(&m_unk0x1c, sizeof(m_unk0x1c));
		p_legoFile->Read(&m_color1, sizeof(m_color1));
		p_legoFile->Read(&m_color2, sizeof(m_color2));
		p_legoFile->Read(&m_color3, sizeof(m_color3));
		p_legoFile->Read(&m_color4, sizeof(m_color4));
	}
	else if (p_legoFile->IsWriteMode()) {
		MxU16 write = m_unk0x12;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_unk0x14;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_unk0x16;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_unk0x18;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_unk0x1a;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_unk0x1c;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_color1;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_color2;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_color3;
		p_legoFile->Write(&write, sizeof(m_unk0x12));

		write = m_color4;
		p_legoFile->Write(&write, sizeof(m_unk0x12));
	}

	return SUCCESS;
}
