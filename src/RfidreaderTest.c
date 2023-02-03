#include <alloca.h>
#include <fcntl.h> // Needed for SPI port
#include <inttypes.h>
#include <linux/spi/spidev.h> // Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h> // Needed for SPI port
#include <unistd.h>

int debug = 0;

#define NRSTPD 26
#define DEV "/dev/spidev1.0"
static uint32_t speed = 1000000;

enum { PCD_IDLE = 0x00,
       PCD_AUTHENT = 0x0E,
       PCD_RECEIVE = 0x08,
       PCD_TRANSMIT = 0x04,
       PCD_TRANSCEIVE = 0x0C,
       PCD_RESETPHASE = 0x0F,
       PCD_CALCCRC = 0x03,

       PICC_REQIDL = 0x26,
       PICC_REQALL = 0x52,
       PICC_ANTICOLL = 0x93,
       PICC_SELECTTAG = 0x93,
       PICC_AUTHENT1A = 0x60,
       PICC_AUTHENT1B = 0x61,
       PICC_READ = 0x30,
       PICC_WRITE = 0xA0,
       PICC_DECREMENT = 0xC0,
       PICC_INCREMENT = 0xC1,
       PICC_RESTORE = 0xC2,
       PICC_TRANSFER = 0xB0,
       PICC_HALT = 0x50,
};

enum { Reserved00 = 0x00,
       CommandReg = 0x01,
       CommIEnReg = 0x02,
       DivlEnReg = 0x03,
       CommIrqReg = 0x04,
       DivIrqReg = 0x05,
       ErrorReg = 0x06,
       Status1Reg = 0x07,
       Status2Reg = 0x08,
       FIFODataReg = 0x09,
       FIFOLevelReg = 0x0A,
       WaterLevelReg = 0x0B,
       ControlReg = 0x0C,
       BitFramingReg = 0x0D,
       CollReg = 0x0E,
       Reserved01 = 0x0F,

       Reserved10 = 0x10,
       ModeReg = 0x11,
       TxModeReg = 0x12,
       RxModeReg = 0x13,
       TxControlReg = 0x14,
       TxAutoReg = 0x15,
       TxSelReg = 0x16,
       RxSelReg = 0x17,
       RxThresholdReg = 0x18,
       DemodReg = 0x19,
       Reserved11 = 0x1A,
       Reserved12 = 0x1B,
       MifareReg = 0x1C,
       Reserved13 = 0x1D,
       Reserved14 = 0x1E,
       SerialSpeedReg = 0x1F,

       Reserved20 = 0x20,
       CRCResultRegM = 0x21,
       CRCResultRegL = 0x22,
       Reserved21 = 0x23,
       ModWidthReg = 0x24,
       Reserved22 = 0x25,
       RFCfgReg = 0x26,
       GsNReg = 0x27,
       CWGsPReg = 0x28,
       ModGsPReg = 0x29,
       TModeReg = 0x2A,
       TPrescalerReg = 0x2B,
       TReloadRegH = 0x2C,
       TReloadRegL = 0x2D,
       TCounterValueRegH = 0x2E,
       TCounterValueRegL = 0x2F,

       Reserved30 = 0x30,
       TestSel1Reg = 0x31,
       TestSel2Reg = 0x32,
       TestPinEnReg = 0x33,
       TestPinValueReg = 0x34,
       TestBusReg = 0x35,
       AutoTestReg = 0x36,
       VersionReg = 0x37,
       AnalogTestReg = 0x38,
       TestDAC1Reg = 0x39,
       TestDAC2Reg = 0x3A,
       TestADCReg = 0x3B,
       Reserved31 = 0x3C,
       Reserved32 = 0x3D,
       Reserved33 = 0x3E,
       Reserved34 = 0x3F,
};

enum { MI_OK = 0,
       MI_NOTAGERR = 1,
       MI_ERR = 2,
};

int MFRC522_Init(unsigned int ce);
int MFRC522_Request(unsigned char reqMode, unsigned char *backBitsp);
int MFRC522_Anticoll(unsigned char **backDatap);
int MFRC522_SelectTag(unsigned char *uid);
int MFRC522_Auth(unsigned char authMode, int BlockAddr,
		 unsigned char *Sectorkey, size_t SectorkeyLen,
		 unsigned char *serNum);
void MFRC522_Read(unsigned char blockAddr);
void MFRC522_Write(unsigned char blockAddr, unsigned char *writeData,
		   size_t writeDataLen);
void MFRC522_StopCrypto1(void);

int openSPI(const char *device, uint32_t speed)
{
	uint8_t mode = SPI_MODE_0;
	uint8_t bits = 8;
	int fd;

	/* Device oeffen */
	if ((fd = open(device, O_RDWR)) < 0) {
		perror("Fehler Open Device");
		return -1;
	}

	/* Mode setzen */
	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
		perror("Fehler Set SPI-Modus");
		close(fd);
		return -1;
	}

	/* Wortlaenge setzen */
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
		perror("Fehler Set Wortlaenge");
		close(fd);
		return -1;
	}

	/* Datenrate setzen */
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		perror("Fehler Set Speed");
		close(fd);
		return -1;
	}

	return fd;
}

void spi_transfer(int fd, unsigned char *data, unsigned int length)
{
	struct spi_ioc_transfer *spi;
	int i;
	uint8_t bits;
	uint32_t speed;

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) == -1) {
		perror("SPI_IOC_RD_BITS_PER_WORD");
		return;
	}
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) == -1) {
		perror("SPI_IOC_RD_MAX_SPEED_HZ");
		return;
	}

	spi = alloca(length * sizeof(struct spi_ioc_transfer));
	memset(spi, 0, length * sizeof(spi[0]));

	for (i = 0; i < length; i++) {
		spi[i].tx_buf = (unsigned long)&data[i];
		spi[i].rx_buf = (unsigned long)&data[i];
		spi[i].len = sizeof(unsigned char);
		spi[i].delay_usecs = 0;
		spi[i].speed_hz = speed;
		spi[i].bits_per_word = bits;
		spi[i].cs_change = 0;
	}

	if (ioctl(fd, SPI_IOC_MESSAGE(length), spi) == -1) {
		perror("SPI_IOC_MESSAGE");
		return;
	}
}

#define MAX_LEN 16



int GPIO_setup(unsigned int pin, int value);
int openSPI(const char *device, uint32_t speed);
void spi_transfer(int fd, unsigned char *data, unsigned int length);

static int MFRC_fd = -1;

static char *blockaddr2sectorblock(unsigned int blockaddr)
{
	static char sector_block[2 + 1 + 2 + 1];

	if (blockaddr < (32 * 4))
		snprintf(sector_block, sizeof(sector_block), "%u/%u",
			 blockaddr / 4, blockaddr % 4);
	else if (blockaddr < (32 * 4 + (39 - 32 + 1) * 16)) {
		blockaddr -= 32 * 4;
		snprintf(sector_block, sizeof(sector_block), "%u/%u",
			 32 + (blockaddr / 16), blockaddr % 16);
	} else
		snprintf(sector_block, sizeof(sector_block), "n/a");

	return sector_block;
}

int sectorblock2blockaddr(unsigned char sector, unsigned char block)
{
	int blockaddr;

	if (debug)
		fprintf(stderr,
			"sectorblock2blockaddr(sector=%#x, block=%#x)\n",
			sector, block);

	if (sector < 32) {
		if (block < 4)
			blockaddr = sector * 4 + block;
		else
			blockaddr = -1;
	} else if (sector < 40) {
		if (block < 16)
			blockaddr = 32 * 4 + (sector - 32) * 16 + block;
		else
			blockaddr = -1;
	}

	if (debug)
		fprintf(stderr, "    = %d\n", blockaddr);

	return blockaddr;
}

static int Write_MFRC522_register(unsigned int addr, unsigned char val)
{
	unsigned char b[2];

	b[0] = (addr << 1) & 0x7E;
	b[1] = val;

	spi_transfer(MFRC_fd, b, 2);
}

static unsigned char Read_MFRC522_register(unsigned int addr)
{
	unsigned char b[2];

	b[0] = ((addr << 1) & 0x7E) | 0x80;
	b[1] = 0;

	spi_transfer(MFRC_fd, b, 2);

	return b[1];
}

static int MFRC522_Reset()
{
	Write_MFRC522_register(CommandReg, PCD_RESETPHASE);
}

static void AntennaOn()
{
	unsigned char temp;

	temp = Read_MFRC522_register(TxControlReg);
	if ((temp & 0x03) != 0x03)
		Write_MFRC522_register(TxControlReg, temp | 0x03);
}

static void ClearBitMask(unsigned int reg, unsigned char mask)
{
	unsigned char tmp;

	tmp = Read_MFRC522_register(reg);
	Write_MFRC522_register(reg, tmp & (~mask));
}

static void SetBitMask(unsigned int reg, unsigned char mask)
{
	unsigned char tmp;

	tmp = Read_MFRC522_register(reg);
	Write_MFRC522_register(reg, tmp | mask);
}

static int MFRC522_ToCard(unsigned char command, unsigned char *sendData,
			  unsigned int nbytes, unsigned char **backDatap,
			  unsigned int *backLenp)
{
	unsigned char irqEn = 0x00;
	unsigned int waitIRq = 0x00;
	int status = MI_ERR;
	unsigned char *backData = NULL;
	unsigned int backLen = 0;
	int i;
	unsigned char n, lastBits;

	if (command == PCD_AUTHENT) {
		irqEn = 0x12;
		waitIRq = 0x10;
	} else if (command == PCD_TRANSCEIVE) {
		irqEn = 0x77;
		waitIRq = 0x30;
	}

	Write_MFRC522_register(CommIEnReg, irqEn | 0x80);
	ClearBitMask(CommIrqReg, 0x80);
	SetBitMask(FIFOLevelReg, 0x80);

	Write_MFRC522_register(CommandReg, PCD_IDLE);

	for (i = 0; i < nbytes; i++)
		Write_MFRC522_register(FIFODataReg, sendData[i]);

	Write_MFRC522_register(CommandReg, command);
	if (command == PCD_TRANSCEIVE)
		SetBitMask(BitFramingReg, 0x80);

	for (i = 2000;;) {
		n = Read_MFRC522_register(CommIrqReg);
		i--;
		if (!((i != 0) && !(n & 0x01) && !(n & waitIRq)))
			break;
	}

	ClearBitMask(BitFramingReg, 0x80);

	if (i != 0) {
		if ((Read_MFRC522_register(ErrorReg) & 0x1B) == 0x00) {
			status = MI_OK;
			if ((n & irqEn) & 0x01)
				status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE) {
				n = Read_MFRC522_register(FIFOLevelReg);
				lastBits = Read_MFRC522_register(ControlReg) &
					   0x07;
				if (lastBits != 0)
					backLen = (n - 1) * 8 + lastBits;
				else
					backLen = n * 8;
				if (n == 0)
					n = 1;
				else if (n > MAX_LEN)
					n = MAX_LEN;

				backData = calloc(n, sizeof(backData[0]));
				for (i = 0; i < n; i++)
					backData[i] = Read_MFRC522_register(
						FIFODataReg);
			}
		} else
			status = MI_ERR;
	}
	if (backDatap != NULL)
		*backDatap = backData;
	if (backLenp != NULL)
		*backLenp = backLen;

	return status;
}

static void CalulateCRC(unsigned char *pIndata, size_t buflen,
			unsigned char *pOutData)
{
	int i;

	ClearBitMask(DivIrqReg, 0x04);
	SetBitMask(FIFOLevelReg, 0x80);
	for (i = 0; i < buflen; i++)
		Write_MFRC522_register(FIFODataReg, pIndata[i]);
	Write_MFRC522_register(CommandReg, PCD_CALCCRC);
	i = 0xff;
	while (1) {
		unsigned char n;

		n = Read_MFRC522_register(DivIrqReg);
		i--;
		if ((i == 0) || (n & 0x04))
			break;
	}

	pOutData[0] = Read_MFRC522_register(CRCResultRegL);
	pOutData[1] = Read_MFRC522_register(CRCResultRegM);
}

int MFRC522_Init(unsigned int ce)
{
	char device[128];

	GPIO_setup(NRSTPD, 1);

	snprintf(device, sizeof(device), DEV, ce);

	MFRC_fd = openSPI(device, speed);

	if (MFRC_fd == -1)
		return 0;

	MFRC522_Reset();

	Write_MFRC522_register(TModeReg, 0x8D);
	Write_MFRC522_register(TPrescalerReg, 0x3E);
	Write_MFRC522_register(TReloadRegL, 30);
	Write_MFRC522_register(TReloadRegH, 0);

	Write_MFRC522_register(TxAutoReg, 0x40);
	Write_MFRC522_register(ModeReg, 0x3D);

	AntennaOn();

	return 1;
}

int MFRC522_Anticoll(unsigned char **backDatap)
{
	unsigned char cmd[2];
	unsigned char *backData;
	unsigned int backLen;
	int status;

	Write_MFRC522_register(BitFramingReg, 0x00);

	cmd[0] = PICC_ANTICOLL; /* 0x93 */
	cmd[1] = 0x20; /* 2 bytes (cmd and this byte) and 0 extra bits */

	status = MFRC522_ToCard(PCD_TRANSCEIVE, cmd, sizeof(cmd), &backData,
				&backLen);

	if (status == MI_OK) {
		int i;

		if (debug)
			fprintf(stderr, "    backLen=%#x\n", backLen);
		if (((backLen + 7) / 8) == 5) {
			unsigned char serNumCheck = 0;

			for (i = 0; i < 5; i++)
				serNumCheck ^= backData[i];

			if (serNumCheck != 0)
				status = MI_ERR;
		} else
			status = MI_ERR;
	}

	*backDatap = backData;

	return status;
}

int MFRC522_Request(unsigned char reqMode, unsigned char *backBitsp)
{
	int status = 0;
	unsigned char TagType[] = { reqMode };
	unsigned char *backData;
	unsigned int backBits;

	Write_MFRC522_register(BitFramingReg, 0x07);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, &backData,
				&backBits);

	if ((status != MI_OK) | (backBits != 0x10))
		status = MI_ERR;

	if (backBitsp != NULL)
		*backBitsp = backBits;

	return status;
}

static void my_dump(void *buf, size_t buflen)
{
	char *sep;
	unsigned char *dp = (unsigned char *)buf;

	for (sep = ""; buflen > 0; sep = ",", buflen--)
		fprintf(stderr, "%s%02x", sep, *dp++);

	return;
}

int MFRC522_SelectTag(unsigned char *serNum)
{
	unsigned char buf[1 + 1 + 5 + 2];
	int i, status;
	unsigned char *backData;
	unsigned int backLen;

	if (debug) {
		fprintf(stderr, "MFRC522_SelectTag(");
		my_dump(serNum, 5);
		fprintf(stderr, ")\n");
	}

	buf[0] = PICC_SELECTTAG; /* command code */
	buf[1] =
		0x70; /* 7 bytes (command code, this byte and serNum) and 0 bits */
	memcpy(&buf[2], serNum, 5);
	CalulateCRC(buf, sizeof(buf) - 2, &buf[2 + 5]);

	status = MFRC522_ToCard(PCD_TRANSCEIVE, buf, sizeof(buf), &backData,
				&backLen);

	if ((status == MI_OK) && (backLen == 0x18)) {
		if (debug)
			fprintf(stderr, "SAK response: %#x\n", backData[0]);
		return backData[0];
	} else
		return 0;
}

int MFRC522_Auth(unsigned char authMode, int BlockAddr,
		 unsigned char *Sectorkey, size_t SectorkeyLen,
		 unsigned char *serNum)
{
	unsigned char *buff;
	size_t buffLen;
	unsigned char *backData;
	unsigned int backLen;
	int status;

	if (debug)
		fprintf(stderr,
			"MFRC522_Auth(authMode=%#x, BlockAddr=%#x, ...)\n",
			authMode, BlockAddr);

	buffLen = 2 + SectorkeyLen + 4;
	buff = alloca(buffLen);

	buff[0] = authMode; // First byte should be the authMode (A or B)
	buff[1] = BlockAddr; // Second byte is the trailerBlock (usually 7)

	memcpy(&buff[2], Sectorkey, SectorkeyLen);

	memcpy(&buff[2 + SectorkeyLen], serNum, 4);

	if (debug) {
		fprintf(stderr, "    buff=[");
		my_dump(buff, buffLen);
		fprintf(stderr, "]\n");
	}
	status =
		MFRC522_ToCard(PCD_AUTHENT, buff, buffLen, &backData, &backLen);

	if (status != MI_OK)
		fprintf(stderr, "AUTH ERROR!!\n");
	if ((Read_MFRC522_register(Status2Reg) & 0x08) == 0)
		fprintf(stderr, "AUTH ERROR(status2reg & 0x08) != 0\n");
	return status;
}

void MFRC522_StopCrypto1()
{
	ClearBitMask(Status2Reg, 0x08);
}

void MFRC522_Read(unsigned char blockAddr)
{
	unsigned char recvData[] = {
		PICC_READ, blockAddr, 0x00, 0x00 // CRC
	};
	int status;
	unsigned char *backData;
	unsigned int backLen;

	fprintf(stderr, "MFRC522_Read(%u)\n", blockAddr);
	CalulateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, sizeof(recvData),
				&backData, &backLen);

	if (backLen == (16 + 2) * 8) /* 16 bytes data, 2 bytes CRC */
	{
		int i;

		fprintf(stderr, "Sector %s", blockaddr2sectorblock(blockAddr));
		for (i = 0; i < 16; i++)
			fprintf(stderr, " %02x", backData[i]);
		fprintf(stderr, " - %02x %02x", backData[16], backData[17]);

		fprintf(stderr, "\n");
	}

	return;
}

void MFRC522_Write(unsigned char blockAddr, unsigned char *writeData,
		   size_t writeDataLen)
{
	unsigned char buff[] = { PICC_WRITE, blockAddr, 0x00, 0x00 };
	int status;
	unsigned char *backData;
	unsigned int backLen;

	CalulateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, sizeof(buff), &backData,
				&backLen);
	if ((status != MI_OK) || (backLen != 4) ||
	    ((backData[0] & 0x0f) != 0x0A))
		status = MI_ERR;
	fprintf(stderr, "%u backData &0x0F == 0x0A %02x\n", backLen,
		backData[0] & 0x0F);
	int i;
	unsigned char *buf;
	//int status;

	buf = alloca(writeDataLen + 2); // 2 extra for CRC

	memcpy(buf, writeData, writeDataLen);
	CalulateCRC(buf, writeDataLen, &buf[writeDataLen]);

	status = MFRC522_ToCard(PCD_TRANSCEIVE, buf, writeDataLen + 2,
				&backData, &backLen);
	return;
}

int GPIO_setup(unsigned int pin, int value)
{
	FILE *fp;
	char path[128];

	/* If the pin is already exported, unexport first */
	if (access(path, F_OK)) {
		snprintf(path, sizeof(path), "/sys/class/gpio/unexport");
		if ((fp = fopen(path, "w")) == NULL) {
			perror(path);
			return 0;
		}
		fprintf(fp, "%d\n", pin);
		fclose(fp);
	}
	/* Now export the pin */
	snprintf(path, sizeof(path), "/sys/class/gpio/export");
	if ((fp = fopen(path, "w")) == NULL) {
		perror(path);
		return 0;
	}
	fprintf(fp, "%d\n", pin);
	fclose(fp);

	usleep(50000); /* sys needs some time to adjust */
	/* Now set direction (and initial value) */
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
	if ((fp = fopen(path, "w")) == NULL) {
		perror(path);
		return 0;
	}
	if (value == -1) {
		fprintf(fp, "in\n");
		fclose(fp);
	} else {
		fprintf(fp, "out\n");
		fclose(fp);
		snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value",
			 pin);
		if ((fp = fopen(path, "w")) == NULL) {
			perror(path);
			return 0;
		}
		fprintf(fp, "%d\n", value ? 1 : 0);
		fclose(fp);
	}
	return 1;
}

void RfidreaderTest()
{
	uint8_t mode, bits;
	int status;
	unsigned char backBits;
	unsigned char *uid;
	unsigned char blockno;
	char *next;

	MFRC522_Init(0);
	while (1) {
		while ((status = MFRC522_Request(PICC_REQIDL, &backBits)) !=
		       MI_OK)
			usleep(500);

		if (status == MI_OK)
			printf("Card detected\n");

		status = MFRC522_Anticoll(&uid);
		if (status == MI_OK) {
			printf("%02x %02x %02x %02x\n", uid[0], uid[1], uid[2],
			       uid[3]);
			fflush(stdout);
			break;
		}
	}
}