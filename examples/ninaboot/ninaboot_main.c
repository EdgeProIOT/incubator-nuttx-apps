/****************************************************************************
 * apps/examples/serialrx/ninaboot_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <nuttx/arch.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * serialrx_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fdin, fdout, fdgpio0, fdgpio1;
  int ret;
  bool eof = false;
  FAR char *buf;
  FAR char *devpathin, *devpathout;

  if (argc == 1)
    {
      devpathin = EXAMPLES_NINABOOT_DEVPATH_IN;
      devpathout = EXAMPLES_NINABOOT_DEVPATH_OUT;
    }
  else if (argc == 2)
    {
      devpathin = argv[1];
      devpathout = EXAMPLES_NINABOOT_DEVPATH_OUT;
    }
  else if (argc == 3)
    {
          devpathin = argv[1];
          devpathout = argv[2];
    }
  else
    {
      fprintf(stderr, "Usage: %s [devpathin] [devpathout]\n", argv[0]);
      goto errout;
    }

  buf = (FAR char *)malloc(CONFIG_EXAMPLES_NINABOOT_BUFSIZE);

  if (buf == NULL)
    {
      fprintf(stderr, "ERROR: malloc failed: %d\n", errno);
      goto errout;
    }

  fdin = open(devpathin, O_RDWR);
  if (fdin < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  fdout = open(devpathout, O_RDWR);
  if (fdout < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  fdgpio0 = open("/dev/gpio0", O_RDWR);
  if (fdgpio0 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  fdgpio1 = open("/dev/gpio1", O_RDWR);
  if (fdgpio1 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  printf("Bridging %s with %s\n", devpathin, devpathout;
  fflush(stdout);

  ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);
  up_udelay(1000);
  ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)0);
  up_udelay(1000);
  ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)1);
  up_udelay(1000000);

  while (1)
    {
      ssize_t n = read(fdin, buf, CONFIG_EXAMPLES_SERIALRX_BUFSIZE);
      ret = write(fdout, buf, n);
      if (n < 0 || ret < 0)
        {
          printf("read failed: %d\n", errno);
          fflush(stdout);
          break;
        }

      ssize_t n = read(fdout, buf, CONFIG_EXAMPLES_SERIALRX_BUFSIZE);
      ret = write(fdin, buf, n);
      if (n < 0 || ret < 0)
        {
          printf("read failed: %d\n", errno);
          fflush(stdout);
          break;
        }
      up_udelay(1000);
    }

  printf("Terminated\n");
  fflush(stdout);

  up_udelay(1000000);
  close(fdin);
  close(fdout);

  free(buf);
  return EXIT_SUCCESS;

errout_with_buf:
  free(buf);

errout:
  fflush(stderr);
  return EXIT_FAILURE;
}
