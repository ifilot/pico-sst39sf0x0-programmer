# -*- coding: utf-8 -*-

import unittest
import serial
import serial.tools.list_ports

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
        self.assertEqual(rsp, bytearray([0xBF,0xB7]))
        
    def test_read_block(self):
        """
        Test reading board id
        """
        # load comparison data
        f = open('F:/Dropbox/Projects/P2000T/bundle.bin', 'rb')
        compdata = bytearray(f.read())
        f.close()
        
        romdata = bytearray()
        for i in range(0, 64):
            self.ser.write(b'RDBK%04X' % i)
            rsp = self.ser.read(8)
            rsp = self.ser.read(256)
            romdata += rsp
        
        self.assertEqual(romdata, compdata[0:len(romdata)])
        
    def test_read_bank(self):
        """
        Test reading board id
        """
        # load comparison data
        f = open('F:/Dropbox/Projects/P2000T/bundle.bin', 'rb')
        compdata = bytearray(f.read())
        f.close()
        
        romdata = bytearray()
        for i in range(0,2):
            self.ser.write(b'RDBANK%02X' % i)
            rsp = self.ser.read(8)
            rsp = self.ser.read(0x4000)
            romdata += rsp
        
        self.assertEqual(romdata, compdata[0:len(romdata)])
        
    def test_read_block_secondary(self):
        """
        Test reading board id
        """
        # load comparison data
        f = open('F:/Dropbox/Projects/P2000T/bundle.bin', 'rb')
        compdata = bytearray(f.read())
        f.close()
        
        romdata = bytearray()
        for i in range(64, 128):
            self.ser.write(b'RDBK%04X' % i)
            rsp = self.ser.read(8)
            rsp = self.ser.read(256)
            romdata += rsp
        
        self.assertEqual(romdata, compdata[64*256:64*256+len(romdata)])
        
if __name__ == '__main__':
    unittest.main()