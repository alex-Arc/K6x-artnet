#include "DMAChannel.h"

#define RXSIZE 12
#define TXSIZE 10
#define BUFLEN 128

typedef struct {
  uint16_t length;
  uint16_t flags;
  void *buffer;
  uint32_t moreflags;
  uint16_t checksum;
  uint8_t ipProtocol;
  uint8_t etherType;
  uint32_t dmadone;
  uint32_t timestamp;
  uint32_t unused1;
  uint32_t unused2;
} enetbufferdesc_t;

static uint8_t _mac[6];

static uint8_t _rxNum;
static uint8_t _txNum;

static enetbufferdesc_t rxRing[RXSIZE] __attribute__ ((aligned(16)));
static enetbufferdesc_t txRing[TXSIZE] __attribute__ ((aligned(16)));

static uint32_t rxBuffer[RXSIZE*BUFLEN] __attribute__ ((aligned(16)));
static uint32_t txBuffer[TXSIZE*BUFLEN] __attribute__ ((aligned(16)));

void init(const uint8_t * macAddrd) {

  memcpy(_mac, macAddrd, 6);

  MPU_RGDAAC0 |= 0x007C0000;
	SIM_SCGC2 |= SIM_SCGC2_ENET;
	CORE_PIN3_CONFIG =  PORT_PCR_MUX(4); // RXD1
	CORE_PIN4_CONFIG =  PORT_PCR_MUX(4); // RXD0
	CORE_PIN24_CONFIG = PORT_PCR_MUX(2); // REFCLK
	CORE_PIN25_CONFIG = PORT_PCR_MUX(4); // RXER
	CORE_PIN26_CONFIG = PORT_PCR_MUX(4); // RXDV
	CORE_PIN27_CONFIG = PORT_PCR_MUX(4); // TXEN
	CORE_PIN28_CONFIG = PORT_PCR_MUX(4); // TXD0
	CORE_PIN39_CONFIG = PORT_PCR_MUX(4); // TXD1
	CORE_PIN16_CONFIG = PORT_PCR_MUX(4); // MDIO
	CORE_PIN17_CONFIG = PORT_PCR_MUX(4); // MDC
	SIM_SOPT2 |= SIM_SOPT2_RMIISRC | SIM_SOPT2_TIMESRC(3);
	// ENET_EIR	1356	Interrupt Event Register
	// ENET_EIMR	1359	Interrupt Mask Register
	// ENET_RDAR	1362	Receive Descriptor Active Register
	// ENET_TDAR	1363	Transmit Descriptor Active Register
	// ENET_ECR	1363	Ethernet Control Register
	// ENET_RCR	1369	Receive Control Register
	// ENET_TCR	1372	Transmit Control Register
	// ENET_PALR/UR	1374	Physical Address
	// ENET_RDSR	1378	Receive Descriptor Ring Start
	// ENET_TDSR	1379	Transmit Buffer Descriptor Ring
	// ENET_MRBR	1380	Maximum Receive Buffer Size
	//		1457	receive buffer descriptor
	//		1461	transmit buffer descriptor

	memset(rxRing, 0, sizeof(rxRing));
	memset(txRing, 0, sizeof(txRing));

  memset(rxBuffer, 0, sizeof(rxBuffer));
	memset(txBuffer, 0, sizeof(txBuffer));

  for (int i=0; i < RXSIZE; i++) {
		rxRing[i].flags = 0x8000; // empty flag
		rxRing[i].buffer = rxBuffer + i * BUFLEN;
	}
	rxRing[RXSIZE-1].flags = 0xA000; // empty & wrap flags
	for (int i=0; i < TXSIZE; i++) {
	  txRing[i].buffer = txBuffer + i * BUFLEN;
	}
	txRing[TXSIZE-1].flags = 0x2000; // wrap flag

	ENET_EIMR = 0;
	ENET_MSCR = ENET_MSCR_MII_SPEED(15);  // 12 is fastest which seems to work
	ENET_RCR = ENET_RCR_NLC | ENET_RCR_MAX_FL(1522) | ENET_RCR_CFEN |
		ENET_RCR_CRCFWD | ENET_RCR_PADEN | ENET_RCR_RMII_MODE |
		/* ENET_RCR_FCE | ENET_RCR_PROM | */ ENET_RCR_MII_MODE;
	ENET_TCR = ENET_TCR_ADDINS | /* ENET_TCR_RFC_PAUSE | ENET_TCR_TFC_PAUSE | */
		ENET_TCR_FDEN;

  uint32_t mac_h = _mac[0]<<2 | _mac[1]<<1 | _mac[2];
  uint32_t mac_l = _mac[3]<<2 | _mac[4]<<1 | _mac[5];

	ENET_PALR = (mac_h << 8) | ((mac_l >> 16) & 255);
	ENET_PAUR = ((mac_l << 16) & 0xFFFF0000) | 0x8808;
	ENET_OPD = 0x10014;
	ENET_IAUR = 0;
	ENET_IALR = 0;
	ENET_GAUR = 0;
	ENET_GALR = 0;
	ENET_RDSR = (uint32_t)rxRing;
	ENET_TDSR = (uint32_t)txRing;
	ENET_MRBR = 512;
	ENET_TACC = ENET_TACC_SHIFT16;
	//ENET_TACC = ENET_TACC_SHIFT16 | ENET_TACC_IPCHK | ENET_TACC_PROCHK;
	ENET_RACC = ENET_RACC_SHIFT16 | ENET_RACC_PADREM;

	ENET_ECR = 0xF0000000 | ENET_ECR_DBSWP | ENET_ECR_EN1588 | ENET_ECR_ETHEREN;
	ENET_RDAR = ENET_RDAR_RDAR;
	ENET_TDAR = ENET_TDAR_TDAR;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("begin");
  uint8_t mac[6] = {0x04, 0xE9, 0xE5, 0x00, 0x00, 0x01};
  init(mac);
}

void loop() {


}
