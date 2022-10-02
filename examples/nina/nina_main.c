/****************************************************************************
 * apps/examples/nina/nina_main.c
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
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <nuttx/arch.h>
#include <nuttx/ioexpander/gpio.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static void show_usage(FAR const char *progname)
{
  fprintf(stderr, "USAGE:\n");
  fprintf(stderr, "\t%s boot|start|on|off\n", progname);
  fprintf(stderr, "Where:\n");
  fprintf(stderr, "\tboot is for boot mode\n");
  fprintf(stderr, "\tstart is for normal boot\n");
  fprintf(stderr, "\ton is for turning module on\n");
  fprintf(stderr, "\toff is for turning module off\n");
}

int main(int argc, FAR char *argv[])
{
  int fdusb;
  int fdnina;
  int fdgpio0;
  int fdgpio1;
  int ret;
  char *buf;
  char *devpathin;
  char *devpathout;
  struct pollfd fds[2];
  struct termios tty;
  int cmd = 0;

  if (argc == 2)
    {
      if (strcmp(argv[1],  "boot") == 0)
        {
          cmd = 0;
        }
      else if (strcmp(argv[1],  "start") == 0)
        {
          cmd = 1;
        }
      else if (strcmp(argv[1],  "on") == 0)
        {
          cmd = 2;
        }
      else if (strcmp(argv[1],  "off") == 0)
        {
          cmd = 3;
        }
      else
        {
          show_usage(argv[0]);
          goto errout;
        }
    }
  else
    {
      show_usage(argv[0]);
      goto errout;
    }

  fdgpio0 = open("/dev/gpio0", O_RDWR);

  if (fdgpio0 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout;
    }

  fdgpio1 = open("/dev/gpio1", O_RDWR);

  if (fdgpio1 < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      close(fdgpio0);
      goto errout;
    }

  if (cmd == 0)
    {
      /* BOOT MODE */

      ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)0);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);
      up_udelay(1000);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)1);
    }
  else if (cmd == 1)
    {
      /* NORMAL BOOT */

      ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)1);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);
      up_udelay(1000);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)1);
    }
  else if (cmd == 2)
    {
      /* ON */

      ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)1);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);
      up_udelay(1000);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)1);

      close(fdgpio0);
      close(fdgpio1);

      return EXIT_SUCCESS;
    }
  else if (cmd == 3)
    {
      /* OFF */

      ret = ioctl(fdgpio1, GPIOC_WRITE, (unsigned long)1);
      ret = ioctl(fdgpio0, GPIOC_WRITE, (unsigned long)0);

      close(fdgpio0);
      close(fdgpio1);

      return EXIT_SUCCESS;
    }

  devpathin = CONFIG_EXAMPLES_NINA_CONSOLE_DEVPATH;
  devpathout = CONFIG_EXAMPLES_NINA_MODULE_DEVPATH;

  buf = (char *)malloc(CONFIG_EXAMPLES_NINA_BUFSIZE);
  memset(buf, 0, CONFIG_EXAMPLES_NINA_BUFSIZE);

  if (buf == NULL)
    {
      fprintf(stderr, "ERROR: malloc failed: %d\n", errno);
      goto errout;
    }

  fdusb = open(devpathin, O_RDWR | O_NONBLOCK);

  if (fdusb < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      goto errout_with_buf;
    }

  if (cmd == 0)
    {
      ret = tcgetattr(fdusb, &tty);

      if (ret < 0)
        {
          printf("ERROR: Failed to get termios: %s\n", strerror(errno));
          close(fdusb);
          goto errout_with_buf;
        }

      tty.c_oflag &= ~OPOST;

      ret = tcsetattr(fdusb, TCSANOW, &tty);

      if (ret < 0)
        {
          printf("ERROR: Failed to set termios: %s\n", strerror(errno));
          close(fdusb);
          goto errout_with_buf;
        }
    }

  fds[0].fd = fdusb;
  fds[0].events = POLLIN;

  fdnina = open(devpathout, O_RDWR | O_NONBLOCK);

  if (fdnina < 0)
    {
      fprintf(stderr, "ERROR: open failed: %d\n", errno);
      close(fdusb);
      goto errout_with_buf;
    }

  fds[1].fd = fdnina;
  fds[1].events = POLLIN;

  fflush(stdout);

  while (1)
    {
      poll(fds, 2, -1);

      if (fds[0].revents & POLLIN)
        {
          ret = read(fdusb, buf, CONFIG_EXAMPLES_NINA_BUFSIZE);

          if (ret > 0)
            {
              write(fdnina, buf, ret);
            }
          else if (ret < 0)
            {
              printf("read USB failed: %d\n", errno);
              fflush(stdout);
              break;
            }
        }

      if (fds[1].revents & POLLIN)
        {
          ret = read(fdnina, buf, CONFIG_EXAMPLES_NINA_BUFSIZE);

          if (ret > 0)
            {
              write(fdusb, buf, ret);
            }
          else if (ret < 0)
            {
              printf("read NINA failed: %d\n", errno);
              fflush(stdout);
              break;
            }
        }
    }

  printf("Terminated\n");
  fflush(stdout);

  close(fdusb);
  close(fdnina);
  close(fdgpio0);
  close(fdgpio1);

  free(buf);
  return EXIT_SUCCESS;

errout_with_buf:
  close(fdgpio0);
  close(fdgpio1);
  free(buf);

errout:
  fflush(stderr);
  return EXIT_FAILURE;
}
