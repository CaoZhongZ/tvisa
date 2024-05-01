#include "sycl_misc.hpp"

#define queue_case(x) \
case x: \
  if (nsub == 0) \
    return getQueue<x, 0>(); \
  else \
    return getQueue<x, 1>();

sycl::queue currentQueue(int ndev, int nsub) {
  switch(ndev) {
    queue_case(0);
    queue_case(1);
    queue_case(2);
    queue_case(3);
    queue_case(4);
    queue_case(5);
    queue_case(6);
    queue_case(7);
  }
  throw std::exception();
}

#define subdev_case(x) \
case x: \
  if (nsub == 0) \
    return getSubDevice<x, 0>(); \
  else \
    return getSubDevice<x, 1>();

sycl::device currentSubDevice(int ndev, int nsub) {
  switch(ndev) {
    subdev_case(0);
    subdev_case(1);
    subdev_case(2);
    subdev_case(3);
    subdev_case(4);
    subdev_case(5);
    subdev_case(6);
    subdev_case(7);
  }
  throw std::exception();
}

static uint32_t g_dev_num = 1;
static uint32_t g_part_num = 0;

sycl::device currentSubDevice() {
  return currentSubDevice(g_dev_num, g_part_num);
}

sycl::queue currentQueue() {
  return currentQueue(g_dev_num, g_part_num);
}
