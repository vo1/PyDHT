from distutils.core import setup, Extension

pydht = Extension('PyDHT',
					include_dirs = ['/usr/local/include'],
					libraries = ['bcm2835'],
					library_dirs = ['/usr/local/lib'],
                    sources = ['src/PyDHT.c'])

setup (name = 'PyDHT',
       version = '0.0.6',
       description = 'PyDHT based on Adafruit Raspberry-Pi Python Code Library by Limor Fried, Kevin Townsend and Mikey Sklar',
	   author='Vo1',
	   author_email='insyon@gmail.com',
	   license='GPLv2',
       keywords=["raspberry pi", "bcm2835", "gpio", "dht11", "dht22", "am2302", "adafruit"],
	   classifiers=[
	       "Programming Language :: Python",
           "Development Status :: 3 - Alpha",
	       "Topic :: System :: Hardware"
	   ],
	   url='https://github.com/vo1/PyDHT',
	   ext_modules = [pydht])
