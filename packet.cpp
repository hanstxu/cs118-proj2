#include "packet.h"

//Send packet declaration. Take in syn, ack, cid, flags, payload size and set the m_header accordingly.
Packet::Packet() {
	m_seq = 0, m_ack = 0, m_cid = 0, m_flags = 0, m_payload_size = 0;
	memset(m_header, 0, HEADER_SIZE);
	m_payload = NULL;
	m_packet = new unsigned char[HEADER_SIZE];;
	memset(m_packet, 0, HEADER_SIZE);
}

Packet::Packet(uint32_t seq, uint32_t ack, uint16_t cid, uint16_t flags,
	unsigned int payload_size) {
	m_seq = 0, m_ack = 0, m_cid = 0, m_flags = 0;
	convert_to_buffer(m_header, seq, ack, cid, flags);
	m_payload_size = payload_size;
	if (payload_size == 0)
		m_payload = NULL;
	else
		m_payload = new unsigned char[payload_size];
	m_packet = new unsigned char[payload_size + HEADER_SIZE];
}

//Receive packet declaration. Take in buffer and set the m_syn, m_ack, m_cid, m_flags accordingly.
Packet::Packet(unsigned char* buffer, unsigned int payload_size) {
	memset(m_header, 0, HEADER_SIZE);
	get_header_info(buffer, m_seq, m_ack, m_cid, m_flags);
	m_payload_size = payload_size;
	if (payload_size == 0)
		m_payload = NULL;
	else {
		m_payload = new unsigned char[payload_size];
		memcpy(m_payload, &buffer[12], m_payload_size);
	}
	m_packet = new unsigned char[payload_size + HEADER_SIZE];
}

Packet& Packet::operator=(const Packet& other) {
	m_seq = other.m_seq;
	m_ack = other.m_ack;
	m_cid = other.m_cid;
	m_flags = other.m_flags;
	m_payload_size = other.m_payload_size;
	memcpy(m_header, other.m_header, HEADER_SIZE);
	
	if (m_payload != NULL)
		delete m_payload;
	delete m_packet;
	
	if (m_payload_size == 0)
		m_payload = NULL;
	else
		m_payload = new unsigned char[m_payload_size];
	m_packet = new unsigned char[m_payload_size + HEADER_SIZE];
	
	memcpy(m_payload, other.m_payload, m_payload_size);
	memcpy(m_packet, other.m_packet, m_payload_size + HEADER_SIZE);
	return *this;
}

Packet::~Packet() {
	delete m_payload;
	delete m_packet;
}

void Packet::set_packet(unsigned char* buffer) {
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
	cout << "Sequence Number: " << m_seq << endl;
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

void Packet::read_payload() const {
	printf("%s", m_payload);
}

unsigned char* Packet::get_buffer() const {
	return m_packet;
}

unsigned char* Packet::get_payload() const {
	return m_payload;
}

unsigned int Packet::get_seq() const {
	return m_seq;
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

unsigned int Packet::get_size() const {
	return m_payload_size + HEADER_SIZE;
}