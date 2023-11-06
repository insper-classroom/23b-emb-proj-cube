import pyautogui
import serial
import argparse
import time
import logging
import time
# import rtmidi
import mido
import sys


#[NAO PERDER ESSE LINK]
# https://spotlightkid.github.io/python-rtmidi/rtmidi.html#module-rtmidi.midiutil


class MyControllerMap:
    def __init__(self):
        self.button = {'A': 'L'} # Fast forward (10 seg) pro Youtube

class SerialControllerInterface:
    # Protocolo
    # byte 1 -> Botão 1 (estado - Apertado 1 ou não 0)
    # byte 2 -> EOP - End of Packet -> valor reservado 'X'

    def __init__(self, port, baudrate):

        self.ser = serial.Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        self.incoming = '0'
        self.controller_number = 0
        self.nota = 60
        self.nota_antiga = 60
        self.pitch = 0
        self.flag = 0
        self.channel = 1
        self.isHandshake = False
        self.note_on = [mido.Message('note_on', note=60, velocity=64), [0x90, 60, 112]]
        self.cchange = [0xB0 + self.channel, self.controller_number, 100]
        self.note_off = 0
        self.data_conf = ['0',[0x80, 60, 0]]
        self.after_touch = [0xA0, 60, 100]
        self.last_mode = ['note_on', 60, 100]
        # self.available_ports = midiout.get_ports()
        self.output = mido.open_output("loopMIDI Port 3")
        self.i = 0
        
        
        pyautogui.PAUSE = 0  ## remove delay

    def make_note(self, nota, estado, velocity, antiga):
        if self.nota != self.nota_antiga:
            self.output.send(mido.Message('note_off', note=self.nota_antiga, velocity=velocity))
            self.output.send(mido.Message('note_on', note=self.nota, velocity=velocity))
        else:
            # self.output.send(mido.Message('note_on', note = self.nota, velocity=velocity))
            pass
    

    def define_nota(self, massage):
        if massage == b'000':
            self.nota = 60
        if massage == b'001':
            self.nota = 63
        if massage == b'010':
            self.nota = 64
        if massage == b'011':
            self.nota = 65
        if massage == b'100':
            self.nota = 66
        if massage == b'101':
            self.nota = 68
        if massage == b'110':
            self.nota = 70
        if massage == b'111':
            self.nota = 72


    # def compara(self, )



    def make_cutoff(self, massage):
        control_change_number = 73
        if massage == b'0000':
            cutoff_value = 0
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'0001':
            cutoff_value = 9
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))            
        elif massage == b'0010':
            cutoff_value = 18
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0011':
            cutoff_value = 27
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0100':
            cutoff_value = 36
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0101':
            cutoff_value = 45
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0110':
            cutoff_value = 54
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'0111':
            cutoff_value = 63
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1000':
            cutoff_value = 56
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1001':
            cutoff_value = 63
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1010':
            cutoff_value = 64
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1011':
            cutoff_value = 77
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1100':
            cutoff_value = 84
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'1101':
            cutoff_value = 93
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))






    def make_atk(self, massage):
        control_change_number = 72
        if massage == b'0000':
            cutoff_value = 127
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'0001':
            cutoff_value = 40
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))            
        elif massage == b'0010':
            cutoff_value = 50
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0011':
            cutoff_value = 55
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0100':
            cutoff_value = 60
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0101':
            cutoff_value = 65
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0110':
            cutoff_value = 70
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'0111':
            cutoff_value = 75
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1000':
            cutoff_value = 85
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1001':
            cutoff_value = 90
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1010':
            cutoff_value = 95
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1011':
            cutoff_value = 100
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1100':
            cutoff_value = 110
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'1101':
            cutoff_value = 125
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))

    #def regra(self, massage):



    def make_dekay(self, massage):
        
        control_change_number = 71
        if massage == b'0000':
            cutoff_value = 0
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'0001':
            cutoff_value = 9
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))            
        elif massage == b'0010':
            cutoff_value = 18
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0011':
            cutoff_value = 27
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0100':
            cutoff_value = 36
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0101':
            cutoff_value = 45
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))  
        elif massage == b'0110':
            cutoff_value = 54
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'0111':
            cutoff_value = 63
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1000':
            cutoff_value = 56
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1001':
            cutoff_value = 63
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1010':
            cutoff_value = 72
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1011':
            cutoff_value = 77
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value)) 
        elif massage == b'1100':
            cutoff_value = 84
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))
        elif massage == b'1101':
            cutoff_value = 93
            self.output.send(mido.Message('control_change', control=control_change_number, value=cutoff_value))

    def make_pitch(self, massage):
        control_change_number = 0x0E
        if massage == b'0000':
            cutoff_value = 0
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'0001':
            cutoff_value = 1000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))   
        elif massage == b'0010':
            cutoff_value = 1500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'0011':
            cutoff_value = 2000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0)) 
        elif massage == b'0100':
            cutoff_value = 2500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'0101':
            cutoff_value = 3000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'0110':
            cutoff_value = 3500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'0111':
            cutoff_value = 4000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'1000':
            cutoff_value = 4500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0)) 
        elif massage == b'1001':
            cutoff_value = 5000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'1010':
            cutoff_value = 5500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'1011':
            cutoff_value = 6000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'1100':
            cutoff_value = 6500
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))
        elif massage == b'1101':
            cutoff_value = 7000
            self.output.send(mido.Message('pitchwheel', pitch=cutoff_value, channel=0))

    def handshake(self):
        print(f'recebi: {self.incoming}')
        self.incoming = self.ser.read()
        if self.incoming != b's':
            logging.info(f"Handshake not initialed\nINCOMING: {self.incoming}")
            return False
        self.ser.write(b'c')
        logging.info('enviando c')
        return True

    def update(self):
        ## Sync protocol 
          
        data = self.ser.read(size= 23)
        #print(data)
        # if data == b's':
        #     print('recebi o handshake')
        #    self.ser.write(b'c')   

        while self.incoming != b'X' :
            self.incoming = self.ser.read()
            logging.debug("Received INCOMING: {}".format(self.incoming))
            # print("lendo")


    #     #LEITURA DE DATA
        data = self.ser.read(size=23)
        
        print(data)

        
        logging.debug("Received DATA: {}".format(data))
    

        # print(self.data_conf[1])
        #Compara o estado com o anterior
        if self.data_conf[0][0:8] == data[0:8]:
            if self.data_conf[1] == self.last_mode:
                #print(self.after_touch)
                
                print("entrou")
            #print("legas")
            else:
                pass

        
        print(data)
        if data[0] != 48:
            self.output.send(mido.Message('note_on', note=60, velocity=100))
        if data[1] != 48:
            self.output.send(mido.Message('note_on', note=62, velocity=100))
        if data[2] != 48:
            self.output.send(mido.Message('note_on', note=64, velocity=100))
        if data[11] != 48:
            self.output.send(mido.Message('note_on', note=65, velocity=100))
        if data[13] != 48:
            self.output.send(mido.Message('note_on', note=66, velocity=100))
        if data[12] != 48:
            self.output.send(mido.Message('note_on', note=67, velocity=100))
    #    if data[0:3] != self.data_conf[0][0:3]:
    #        self.define_nota(data[0:3])
            
  
    #    else:
    #        self.nota_antiga = self.nota

        self.make_cutoff(data[3:7])
        self.make_atk(data[8:12])
        self.make_pitch(data[13:17])
        self.make_dekay(data[18:22])

        #Salva no estado anterior
        self.incoming = self.ser.read()
        self.data_conf[0] = data



class DummyControllerInterface:
    def __init__(self):
        self.mapping = MyControllerMap()

    def update(self):
        pyautogui.keyDown(self.mapping.button['A'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['A'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)


if __name__ == '__main__':
    
    interfaces = ['dummy', 'serial']

   
    argparse = argparse.ArgumentParser()
    argparse.add_argument('-b', '--baudrate', type=int, default=115200)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    
    
    
    print("Connection to {} using {} interface ({})".format("com3", args.controller_interface, args.baudrate))
    print(args.baudrate)

    if args.controller_interface == 'dummy':
        controller = DummyControllerInterface()
    else:
        # midiout = rtmidi.MidiOut()
        # midiin = rtmidi.MidiIn()
        controller = SerialControllerInterface(port="COM7", baudrate=args.baudrate)

    while True:
        controller.update()
    self.output.close()
    sys.exit()
