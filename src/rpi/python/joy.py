import pygame

# Definitions
# Left stick
AXIS_A = 1

#right stick
AXIS_X = 2
AXIS_Y = 3

BUT_ST = 4

joy_config = { 'alt': {   'reverse': True,
                        'max': 1023,
                        'min': 10,
                        'map_constant': 1023 
                    },
               'x':   {   'reverse': False,
                        'max': 1023,
                        'min': 10,
                        'map_constant': 1023 
                    },
               'y':   {   'reverse': False,
                        'max': 1023,
                        'min': 10,
                        'map_constant': 1023 
                    }
              }

pygame.init()
size = [1, 1]
screen = pygame.display.set_mode(size)

pygame.joystick.init()
controller = pygame.joystick.Joystick(0)
controller.init()

def set_joy_configs(alt_rev,
                    alt_mp_k,
                    alt_min,
                    alt_max,
                    x_rev,
                    x_mp_k,
                    x_min,
                    x_max,
                    y_rev,
                    y_mp_k,
                    y_min,
                    y_max):

    global joy_config
    joy_config = { 'alt': { 'reverse': alt_rev,
                            'max': alt_max,
                            'min': alt_min,
                            'map_constant': alt_mp_k 
                        },
                   'x':   { 'reverse': x_rev,
                            'max': x_max,
                            'min': x_min,
                            'map_constant': x_mp_k 
                        },
                   'y':   { 'reverse': y_rev,
                            'max': y_max,
                            'min': y_min,
                            'map_constant': y_mp_k 
                        }
                  }

#def _joy_configs()


def normalize(reading, config):
    if config['reverse'] == True:
        reading *= -1
    reading *= config['map_constant']
    reading = int(reading)
    if abs(reading) > config['max']:
        sign = 1
        if reading < 0:
            sign = -1
        reading = config['max'] * sign
    elif abs(reading) < config['min']:
        sign = 1
        if reading < 0:
            sign = -1
        reading = config['min'] * sign
    return reading

def get_joy_readings():
    global joy_config

    pygame.event.get()
    mq_a = normalize( controller.get_axis( AXIS_A ), joy_config['alt'])
    mq_x = normalize( controller.get_axis( AXIS_X ), joy_config['x'])
    mq_y = normalize( controller.get_axis( AXIS_Y ), joy_config['y'])

    # For alt use only positive numbers
    if mq_a < 0:
        mq_a = 0

    mq_s = controller.get_button( BUT_ST ) # L1

    dict_out = { 'joy_a' : mq_a,
                 'joy_x' : mq_x, 
                 'joy_y' : mq_y, 
                 'joy_s' : mq_s }

    print "Joy readings: ", dict_out
    return dict_out

if __name__ == '__main__':

    while True:

        pygame.event.get()

        axis   = []
        button = []

        axes = controller.get_numaxes()
        
        for i in range( axes ):
            axis[i] = controller.get_axis( i )
            
        buttons = controller.get_numbuttons()

        for i in range( buttons ):
            button[i] = controller.get_button( i )
            
        # Hat switch. All or nothing for direction, not like joysticks.
        # Value comes back in an array.
        hats = controller.get_numhats()

        for i in range( hats ):
            hat = controller.get_hat( i )

        col_width = 12
        print "".join(word.ljust(col_width) for word in row )
        sleep(2)
