#include "packet.h"

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

Packet::Packet(unsigned int syn, unsigned int ack, unsigned short cid,
	unsigned short flags, unsigned int payload_size) {
	m_syn = 0, m_ack = 0, m_cid = 0, m_flags = 0;
	convert_to_buffer(m_header, syn, ack, cid, flags);
	m_payload_size = payload_size;
	m_payload = new char[payload_size];
	m_packet = new char[payload_size + HEADER_SIZE];
}

Packet::Packet(char* buffer, unsigned int payload_size) {
	memset(m_header, 0, HEADER_SIZE);
	get_header_info(buffer, m_syn, m_ack, m_cid, m_flags);
	m_payload_size = payload_size;
	m_payload = new char[payload_size];
	m_packet = new char[payload_size + HEADER_SIZE];
}

Packet::~Packet() {
	delete m_payload;
	delete m_packet;
}

void Packet::set_packet(char* buffer) {
	memcpy(m_payload, buffer, m_payload_size);
	memcpy(m_packet, m_header, HEADER_SIZE);
	memcpy(&m_packet[HEADER_SIZE], m_payload, m_payload_size);
}

void Packet::read_buffer() const {
	unsigned int i = 0;
	for (; i < 12; i++)
		printf("%x ", (unsigned char)m_header[i]);
	cout << endl;
	
	for (; i < 12 + m_payload_size; i++)
		printf("%x ", (unsigned char)m_packet[i]);
	cout << endl;
}

void Packet::read_header() const {
	cout << "Sequence Number: " << m_syn << endl;
	cout << "Acknowledgement Number: " << m_ack << endl;
	cout << "Connection ID: " << m_cid << endl;
	cout << "Flags: ";
	if (CHECK_BIT(m_flags, 2))
		cout << "A_Flag ";
	if (CHECK_BIT(m_flags, 1))
		cout << "S_Flag ";
	if (CHECK_BIT(m_flags, 0))
		cout << "F_Flag ";
	cout << endl;
}

char* Packet::get_buffer() const {
	return m_packet;
}

unsigned int Packet::get_syn() const {
	return m_syn;
}

unsigned int Packet::get_ack() const {
	return m_ack;
}

unsigned short Packet::get_cid() const {
	return m_cid;
}

unsigned short Packet::get_flags() const {
	return m_flags;
}