import math
import time
import cv2


#Cambiar "Pepsi.mp4" por el nombre del video que se quiera procesar
cap = cv2.VideoCapture('./Videos/Pepsi.mp4')
f = open('video_converted.txt', 'w')

#f.write("const uint16_t frames[] PROGMEM = {")
#for i in range(0,80):
    #f.write("frame" + str(i) + ", ")

#f.write("};")
#f.write('\n')


cont = 0


while(cap.isOpened()):

    ret, frame = cap.read()

    if ret == True:


        degrees = 0
        degrees_aux = 0
        NUM_LEDS = 200
        HALF_LEDS = int(NUM_LEDS/2)
        row = 0
        resolution = 6
        img_converted = [[0 for k in range(HALF_LEDS)] for l in range(int(360/resolution))]

        while degrees < 360:
            for j in range(0, HALF_LEDS):
                if degrees <= 90:
                    degrees_aux = 90-degrees
                    x = int(HALF_LEDS + (HALF_LEDS-j)*math.sin(math.radians(degrees_aux)))
                    y = int(HALF_LEDS - (HALF_LEDS-j)*math.cos(math.radians(degrees_aux)))
                
                elif degrees <= 180:
                    degrees_aux = degrees - 90
                    x = int(HALF_LEDS - (HALF_LEDS-j)*math.sin(math.radians(degrees_aux)))
                    y = int(HALF_LEDS - (HALF_LEDS-j)*math.cos(math.radians(degrees_aux)))

                elif degrees <= 270:
                    degrees_aux = 270 - degrees
                    x = int(HALF_LEDS - (HALF_LEDS-j)*math.sin(math.radians(degrees_aux)))
                    y = int(HALF_LEDS + (HALF_LEDS-j)*math.cos(math.radians(degrees_aux)))

                elif degrees < 360:
                    degrees_aux = degrees - 270
                    x = int(HALF_LEDS + (HALF_LEDS-j)*math.sin(math.radians(degrees_aux)))
                    y = int(HALF_LEDS + (HALF_LEDS-j)*math.cos(math.radians(degrees_aux)))

                if x>(NUM_LEDS-1):
                    x=NUM_LEDS-1
                if y>(NUM_LEDS-1):
                    y=NUM_LEDS-1

                red8 = frame[x, y, 2]
                green8 = frame[x, y, 1]
                blue8 = frame[x, y, 0]
                
                red5 = (red8*249 + 1014) >> 11
                green6 = (green8*253 + 505) >> 10
                blue5 = (blue8*249 + 1014) >> 11
                
                img_converted[row][j] = (red5<<11) + (green6<<5) + blue5
            
            degrees += resolution
            row += 1


        f.write("const uint16_t frame" + str(cont) + "[][100] PROGMEM = {")
        f.write('\n')

        for i in range(0, len(img_converted)):
            f.write('{')
            for j in range(0, len(img_converted[0])):
                if img_converted[i][j] == 0:
                    f.write("0x0000")

                elif img_converted[i][j] <= 15:
                    hex_number_str = str(hex(img_converted[i][j]))
                    f.write(hex_number_str[:2] + "000" + hex_number_str[2:])
                
                elif img_converted[i][j] <= 255:
                    hex_number_str = str(hex(img_converted[i][j]))
                    f.write(hex_number_str[:2] + "00" + hex_number_str[2:])

                elif img_converted[i][j] <= 4095:
                    hex_number_str = str(hex(img_converted[i][j]))
                    f.write(hex_number_str[:2] + "0" + hex_number_str[2:])

                else:
                    f.write(str(hex(img_converted[i][j])))

                #if j < len(img_converted[0])-1 :
                    #f.write(',')
                f.write(',')

            f.write('}')
            #if i < len(img_converted) - 1:
                #f.write(',')
            f.write(',')
            f.write('\n')
        
        cont += 1
        f.write("};")
        #f.write("},")


        for i in range(0, 5):
            f.write('\n')


    else:
       break

