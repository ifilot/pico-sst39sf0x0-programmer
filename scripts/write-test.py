# -*- coding: utf-8 -*-

import unittest
import serial
import serial.tools.list_ports
import numpy as np

def read_rom(filename):
    f = open(filename, 'rb')
    data = bytearray(f.read())      
    f.close()
    return data

#
# Test board functionality
#
class TestBoard(unittest.TestCase):

    def setUp(self):
        # autofind any available boards
        ports = serial.tools.list_ports.comports()
        portfound = None
        for port in ports:
            if port.pid == 0x0A and port.vid == 0x2E8A:
                portfound = port.device
                break

        # specify the COM port below
        if portfound:
            self.ser = serial.Serial(portfound, 
                                     19200, 
                                     bytesize=serial.EIGHTBITS,
                                     parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE,
                                     timeout=None)  # open serial port
                       
            while(not self.ser.isOpen()):
                self.ser.open()
        else:
            raise Exception('Cannot find suitable port connection')

    def teardDown(self):
        while(not self.ser.isClosed()):
            self.ser.close()

    def test_board_id(self):
        """
        Test reading board id
        """
        self.ser.write(b'READINFO')
        rsp = self.ser.read(8)
        rsp = self.ser.read(16)
        print(rsp)
        self.assertEqual(rsp[0:4], b'PICO')
        
    def test_device_id(self):
        """
        Test reading board id
        """
        self.ser.write(b'DEVIDSST')
        rsp = self.ser.read(8)
        rsp = self.ser.read(2)
        #print(' '.join('0x%02X' % d for d in rsp))
        self.assertEqual(rsp, bytearray([0xBF,0xB7]))
    
    def test_execute_erase(self):
        """
        Test erasing of sector on board
        """
        self.ser.write(b'ESST0000')
        rsp = self.ser.read(8)
        rsp = self.ser.read(2) # read pollbyte count
        print(' '.join('0x%02X' % d for d in rsp))
        
        self.ser.write(b'RDBK0000')
        rsp = self.ser.read(8)
        rsp = self.ser.read(256)
        self.assertEqual(rsp, bytearray([0xFF] * 256))
    
    def test_execute_write(self):
        """
        Test reading board id
        """
        self.ser.write(b'ESST0030')
        rsp = self.ser.read(8)
        print(rsp)
        rsp = self.ser.read(2)
        print(' '.join('0x%02X' % d for d in rsp))
        
        # generate random data
        randomdata = bytearray(np.random.randint(0, 256, 256, dtype=np.uint8))
        self.ser.write(b'WRBK003F')
        rsp = self.ser.read(8)
        print(rsp)
        self.ser.write(randomdata)
        checksum = np.uint8(self.ser.read(1)[0])
        print(np.sum(randomdata) & 0xFF)
        print(np.sum(checksum))
        
        self.ser.write(b'RDBK003F')
        rsp = self.ser.read(8)
        print(rsp)
        rsp = self.ser.read(256)
        #print(' '.join('0x%02X' % d for d in randomdata))
        #print(' '.join('0x%02X' % d for d in rsp))
        self.assertEqual(rsp, randomdata)
        
    def test_write_4k(self):
        """
        Test reading board id
        """
        self.ser.write(b'ESST0050')
        rsp = self.ser.read(8)
        print(rsp)
        rsp = self.ser.read(2)
        print(' '.join('0x%02X' % d for d in rsp))
        
        # check erasing
        res = bytearray()
        for i in range(0,0x1000//256):
            self.ser.write(b'RDBK%04X' % (i + 0x50))
            rsp = self.ser.read(8)
            #print(rsp)
            rsp = self.ser.read(0x100)
            res += rsp
        self.assertEqual(res, bytearray(np.ones(0x1000, dtype=np.uint8) * 0xFF))
               
        # generate random data
        randomdata = bytearray(np.random.randint(0, 256, 0x1000, dtype=np.uint8))
        self.ser.write(b'WRSECT05')
        rsp = self.ser.read(8)
        print(rsp)
        self.ser.write(randomdata)
        crc16checksum = self.ser.read(2)
        
        # assert checksum
        self.assertEqual(int.from_bytes(crc16checksum, byteorder='little', signed=False), 
                         crc16(randomdata))
        
        # check writing
        self.ser.write(b'RDSECT05')
        rsp = self.ser.read(8)
        rsp = self.ser.read(0x1000)
        self.assertEqual(rsp, randomdata)

def crc16(data):
    crc = int(0)
    
    poly = 0x1021
    
    for c in data: # fetch byte
        crc ^= (c << 8) # xor into top byte
        for i in range(8): # prepare to rotate 8 bits
            crc = crc << 1 # rotate
            if crc & 0x10000:
                crc = (crc ^ poly) & 0xFFFF # xor with XMODEN polynomic
    
    return crc        

def printhex(data):
    for i in range(0, len(data)):
        print('%02X ' % data[i], end='')
        if (i+1) % 16 == 0:
            print()
    print()

if __name__ == '__main__':
    unittest.main()