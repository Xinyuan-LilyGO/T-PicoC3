from machine import UART, Pin
import time

class esp_uart:
    def __init__(self,bus_num):
        self.uart = UART(bus_num, baudrate=115200, tx=Pin(8), rx=Pin(9), cts=Pin(10), rts=Pin(11))
        self.uart.write('ATE0\r\n') #turn off echo
        
    def sendAT(self,cmd):
        self.uart.write('AT+'+cmd+'\r\n')
        while self.uart.any()==0:
            time.sleep_ms(1)
        time.sleep_ms(50) #Prevent data loss caused by excessive data
        return print(self.uart.read())
    
        