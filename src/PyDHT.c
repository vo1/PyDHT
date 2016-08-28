#include <bcm2835.h>
#include <Python.h>

#define MAXTIMINGS 100

//#define DEBUG

#define DHT11 11
#define DHT22 22
#define AM2302 22

static PyObject *
PyDHT_init(PyObject *self)
{
    int rtn = bcm2835_init();
    return Py_BuildValue("i",rtn);
}

static PyObject *
PyDHT_close(PyObject *self)
{
    int rtn = bcm2835_close();
    return Py_BuildValue("i",rtn);
}

static PyObject *
PyDHT_read(PyObject *self, PyObject *args)
{
  uint8_t type;
  uint8_t pin;
  uint8_t delay;

  if (!PyArg_ParseTuple(args,"iii",&type,&pin,&delay)) return NULL;

  if (!delay) delay = 500;

  int bits[250], data[100];
  int bitidx = 0;

  int counter = 0;
  int laststate = HIGH;
  int j=0;
  // Set GPIO pin to output
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(pin, HIGH);
  usleep(delay * 1000);  // ms
  bcm2835_gpio_write(pin, LOW);
  usleep(20000); // 20 ms

  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for pin to drop
  uint16_t micros_waited = 0;
  while (bcm2835_gpio_lev(pin) == 1) {
    if (micros_waited == 1000) {
      Py_RETURN_NONE;
    }
    usleep(1);
    ++micros_waited;
  }

  // read data!
  int i;
  for (i = 0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(pin) == laststate) {
	counter++;
	//nanosleep(1);		// overclocking might change this?
        if (counter == 1000)
	  break;
    }
    laststate = bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200)
        data[j/8] |= 1;
      j++;
    }
  }

  if ((j >= 39) &&
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
     // yay!
     if (type == DHT11)
	return Py_BuildValue("[i,i]", data[2], data[0]);
     if (type == DHT22) {
	float f, h;
	h = data[0] * 256 + data[1];
	h /= 10;
	f = (data[2] & 0x7F)* 256 + data[3];
	f /= 10.0;
        if (data[2] & 0x80)  f *= -1;
	return Py_BuildValue("(f,f)", f, h);
    }
    Py_RETURN_NONE;
  }
  Py_RETURN_NONE;
}

static PyMethodDef PyDHTMethods[] = {
    {"init", (PyCFunction)PyDHT_init, METH_NOARGS, "Initialize BCM2835 for DHT."},
    {"read", (PyCFunction)PyDHT_read, METH_VARARGS, "Reads temp/humidity from DHT [sensor] at [pin] with an internal [delay] ms."},
    {"close", (PyCFunction)PyDHT_close, METH_NOARGS, "Close BCM2835 library."}
#if PY_MAJOR_VERSION >= 3
    ,
    {NULL, NULL}
#endif
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef PyDHTModule = {
    PyModuleDef_HEAD_INIT,
    "PyDHT",
    NULL,
    -1,
    PyDHTMethods
};

PyMODINIT_FUNC PyInit_PyDHT(void)
#else
PyMODINIT_FUNC initPyDHT(void)
#endif
{
    PyObject *m;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&PyDHTModule);
#else
    m = Py_InitModule("PyDHT", PyDHTMethods);
#endif
    if (m == NULL)
#if PY_MAJOR_VERSION >= 3
        return NULL;
#else
        return;
#endif

    // Constants
    PyModule_AddIntConstant(m, "DHT11", DHT11);
    PyModule_AddIntConstant(m, "DHT22", DHT22);
    PyModule_AddIntConstant(m, "AM2302", AM2302);

#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
