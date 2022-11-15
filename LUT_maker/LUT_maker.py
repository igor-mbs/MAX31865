#
# Created   : 25/11/2021
# Author	: Igor M B Silva
# e-mail	: igor-mbs@outlook.com
# GitHub	: Angry-Robot
#
import numpy as np
from scipy import optimize
import matplotlib.pyplot as plt
import os

file = open("LUT.txt", "w+")

#os.system('cls')  # on windows
#os.system('clear')  # on linux / os x

#------------------------------------------------------ Model used ---------------------------------------------------------------------#
# ADCcode[k] = (Rcvd[k] * 32768)/Rref
# Rcvd[k] = (ADCcode[k] * Rref)/32768
# Rcvd - (ADCcode * Rref)/32768 = 0
# f(t) : ((r0*( 1 + (a*t) + (b*(t**2)) + (c*(t-100)*(t**3)) )) - ((ADCcode * Rref)/32768)) = 0      # f
# fprime = r0*(a + (2*t*(b + 2*c*(t-75)*t)))                                                        # fprime
# fprime2 = 2*r0*(b + (6*c*(t-50)*t))                                                               # fprime2

#------------------------------------------------------ Constants ---------------------------------------------------------------------#
Rref = np.longdouble(400.0) # Reference resistor value in Ohms
r0 = np.longdouble(100.0)   # Resistance of the platinum RTD at 0ºC in Ohms

maxTemp = np.longdouble(200.0)  # Maximum temperature expected to be used
minTemp = np.longdouble(-30.0)  # Minimum temperature expected to be used

# Callendar–Van Dusen equation coefficients
a = np.longdouble(3.9083)*(10**(-3))
b = np.longdouble(-5.775)*(10**(-7))
c = np.longdouble(-4.18301)*(10**(-12))

#------------------------------------------------------ Functions ---------------------------------------------------------------------#
def celsius_to_ADCcode(temperature):
    if temperature < 0:
        return round( ((r0*( 1 + (a*temperature) + (b*(temperature**2)) + (c*(temperature-100)*(temperature**3)) ))*32768)/Rref )
    else:
        return round( ((r0*( 1 + (a*temperature) + (b*(temperature**2)) ))*32768)/Rref )

def fNegative(t) : 
        return np.longdouble((r0*( 1 + (a*t) + (b*(t**2)) + (c*(t-100)*(t**3)) )) - ((o * Rref)/32768))    # function for the negative part

def fPositive(t) : 
        return np.longdouble((r0*( 1 + (a*t) + (b*(t**2)) )) - ((o * Rref)/32768))   # function for the positive part
        
#------------------------------------------------------ Finding edge values -----------------------------------------------------------#
print(" ")
print(" - Finding edge values - ")
print(" ")

adcMinTemp = celsius_to_ADCcode(minTemp)    # -30 ºC  :   7227 bits
print("ADC register code for minimum temperature: " + str(adcMinTemp))
file.write("ADC register code for minimum temperature: " + str(adcMinTemp) + "\n")

adc0Degree = celsius_to_ADCcode(0)          # 0 ºC   :   8192 bits
print("ADC register code for 0ºC: " + str(adc0Degree))
file.write("ADC register code for 0ºC: " + str(adc0Degree) + "\n")

adcMaxTemp = celsius_to_ADCcode(maxTemp)    # 200 ºC :   14406 bits
print("ADC register code for maximum temperature: " + str(adcMaxTemp))
file.write("ADC register code for maximum temperature: " + str(adcMaxTemp) + "\n" + "\n")

adcNegativeRange    = range( int(adcMinTemp), int(adc0Degree) )
adcPositiveRange    = range( int(adc0Degree), int(adcMaxTemp+2) )
rootTemp            = np.zeros(len(range( int(adcMinTemp), int(adcMaxTemp+2) )))

Rcvd                = np.zeros(len(range( int(adcMinTemp), int(adcMaxTemp+2) )))
ADCcode             = np.zeros(len(range( int(adcMinTemp), int(adcMaxTemp+2) )))

#------------------------------------------------------ Calculating the actual roots ---------------------------------------------------#
print(" ")
print(" - Calculating the actual roots - ")
print(" ")

k=0
# for o in ADCcode:
for o in adcNegativeRange:
    Rkick = (o/32)-256  # Initial guess

    root = optimize.newton( fNegative, Rkick, fprime=lambda t: (r0*(a + (2*t*(b + 2*c*(t-75)*t)))), maxiter = 500 , fprime2=lambda t: (2*r0*(b + (6*c*(t-50)*t))) )
    rootTemp[k] = root

    # Sanity check "prints"
    # Rcvd[k] = r0*( 1 + (a*root) + (b*(root**2)) + (c*(root-100)*(root**3)) )                                                                # Sanity check "prints"
    # ADCcode[k] = ((Rcvd[k])*32768)/400                                                                                                      # Sanity check "prints"
    # printVar = (str(k) + "  ,  " + str(o) + "  ,  " + str(ADCcode[k]) + "  ,  " + str(Rcvd[k]) + "  ,  " + str(rootTemp[k]) + "\n")         # Sanity check "prints"

    printVar = (str(rootTemp[k]) + ",\n")
    file.write(printVar)

    k+=1

for o in adcPositiveRange:
    Rkick = (o/32)-256  # Initial guess

    root = optimize.newton( fPositive, Rkick, fprime=lambda t: (r0*(a + (2*t*(b)))), maxiter = 500 , fprime2=lambda t: (2*r0*(b)) )
    rootTemp[k] = root
    
    # Sanity check "prints"
    # Rcvd[k] = r0*( 1 + (a*root) + (b*(root**2)) )                                                                                    # Sanity check "prints"
    # ADCcode[k] = ((Rcvd[k])*32768)/400                                                                                               # Sanity check "prints"
    # printVar = (str(k) + "  ,  " + str(o) + "  ,  " + str(ADCcode[k]) + "  ,  " + str(Rcvd[k]) + "  ,  " + str(rootTemp[k]) + "\n")  # Sanity check "prints"

    printVar = (str(rootTemp[k]) + ",\n")
    file.write(printVar)

    k+=1

print(" - DONE! :D - ")
print(" ")

