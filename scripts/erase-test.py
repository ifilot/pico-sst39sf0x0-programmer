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
    
    def test_erase(self):
        """
        Test erasing of sector on board
        """
        self.ser.write(b'ERASEALL')
        rsp = self.ser.read(8)
        rsp = self.ser.read(2) # read pollbyte count
        print(' '.join('0x%02X' % d for d in rsp))

if __name__ == '__main__':
    unittest.main()