/* STD */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* GNU, POSIX */
#include <errno.h>
#include <getopt.h>

#define NIB_VERSION 202507L

static bool debug_flag = false;

static struct option lopts[] = { /* Long Options */
  { "debug",   no_argument, NULL, 'd' },
  { "help",    no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { NULL,      no_argument, NULL,  0  }
};

static const char *ldescs[] = { /* Long Descriptions */
  "enable debugging output",
  "display this usage message",
  "display the program version",
  NULL
};

static const char *loptstr = "dhv"; /* Long Option String */

void
usage (char *argv[])
{
  fprintf (stderr, "usage: %s [options] file\n", argv[0]);
  fprintf (stderr, "\toptions:\n");

  for (int idx = 0; lopts[idx].name != NULL; ++idx)
    {
      fprintf (stderr, "\t\t-%c, --%s: %s\n", lopts[idx].name[0],
                                              lopts[idx].name,
                                              ldescs[idx]);
    }
}

#define RAM_N 128 /* Number of RAM Addresses */
#define REG_N 16  /* Number of Registers */

typedef int8_t byte_t; /* Signed 8-bit Binary */

byte_t RAM[RAM_N] = { 0 }; /* Random-access Memory */
byte_t REG[REG_N] = { 0 }; /* General-purpose Registers */

byte_t PC     =   0;      /* Program Counter */
byte_t MAR    =   0;      /* Memory Address Register */
byte_t MBR[2] = { 0, 0 }; /* Memory Buffer Registers */
byte_t CIR[2] = { 0, 0 }; /* Current Instruction Registers */
byte_t SR     =   1;      /* Status Register */

#define SR_SUCCESS  0
#define SR_FAILURE -1

byte_t LHS = 0; /* Left-hand Side of Comparison */
byte_t RHS = 0; /* Right-hand Side of Comparison */

void
debug (const char *fmt, ...)
{
  if (!debug_flag)
    return;

  va_list args;
  va_start (args, fmt);
  fprintf (stderr, "[%s] ", __TIMESTAMP__);
  vfprintf (stderr, fmt, args);
  va_end (args);
}

void
error (const char *str)
{
  fprintf (stderr, "Error: %s\n", str);
  exit (EXIT_FAILURE);
}

void
load (const char *file)
{
  FILE *fp = fopen (file, "rb");

  if (fp == NULL)
    {
      error (strerror (errno));
    }

  size_t n_bytes = fread (RAM, sizeof (byte_t), RAM_N, fp);

  debug ("%s (%zuB)\n", file, n_bytes);

  fclose (fp);
}

int
args (int argc, char *argv[])
{
  int opt; /* Option */
  int ind; /* Index */

  while ((opt = getopt_long (argc, argv, loptstr, lopts, &ind)) != -1)
    {
      switch (opt)
        {
        case 'd':
          debug_flag = true;
          break;

        case 'h':
          usage (argv);
          exit (EXIT_SUCCESS);
          break;

        case 'v':
          printf ("version: %ld\n", NIB_VERSION);
          exit (EXIT_SUCCESS);

        default:
          error (strerror (EINVAL));
        }
    }

  return optind;
}

void
run (void)
{
  uint8_t opc = 0xFF;

  while (SR > 0)
    {
      /* FETCH */

      MAR    = PC;
      MBR[0] = RAM[MAR];
      MBR[1] = RAM[MAR + 1];
      CIR[0] = MBR[0];
      CIR[1] = MBR[1];
      PC    += 2;

      /* DECODE/EXECUTE */

      opc = (CIR[0] >> 4) & 0xF;

      switch (opc)
        {
        case 0x0: /* Halt */
          {
            SR = SR_SUCCESS;

            debug ("Halt\n");
          }
          break;

        case 0x1: /* Read */
          {
            byte_t R  = CIR[0] & 0xF;
            byte_t XY = CIR[1];

            if (XY < 0)
              {
                SR = SR_FAILURE;
                break;
              }

            REG[R] = RAM[XY];

            debug ("%d -> R%d = %d\n", XY, R, REG[R]);
          }
          break;

        case 0x2: /* Write */
          {
            byte_t R  = CIR[0] & 0xF;
            byte_t XY = CIR[1];

            if (XY < 0)
              {
                SR = SR_FAILURE;
                break;
              }

            RAM[XY] = REG[R];

            debug ("R%d -> %d = %d\n", R, XY, REG[R]);
          }
          break;

        case 0x3: /* Copy */
          {
            byte_t R  = CIR[0] & 0xF;
            byte_t XY = CIR[1];

            REG[R] = XY;

            debug ("%d -> R%d = %d\n", XY, R, REG[R]);
          }
          break;
        
        case 0x4: /* Duplicate */
          {
            byte_t R  =  CIR[0] & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;

            REG[R] = REG[S];

            debug ("R%d -> R%d = %d\n", S, R, REG[R]);
          }
          break;

        case 0x5: /* Add */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;
            byte_t T  =  CIR[1]       & 0xF;

            REG[R] = REG[S] + REG[T];

            debug ("R%d + R%d -> R%d = %d\n", S, T, R, REG[R]);
          }
          break;

        case 0x6: /* Subtract */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;
            byte_t T  =  CIR[1]       & 0xF;

            REG[R] = REG[S] - REG[T];

            debug ("R%d - R%d -> R%d = %d\n", S, T, R, REG[R]);
          }
          break;

        case 0x7: /* Shift */
          {
            byte_t R  = CIR[0] & 0xF;
            byte_t XY = CIR[1];

            if (XY >= 0)
              {
                REG[R] <<=  XY;
              }
            else
              {
                REG[R] >>= -XY;
              }

            debug ("R%d << %d = %d\n", R, XY, REG[R]);
          }
          break;

        case 0x8: /* NOT */
          {
            byte_t R =  CIR[0]       & 0xF;
            byte_t S = (CIR[1] >> 4) & 0xF;

            REG[R] = ~REG[S];

            debug ("NOT S%d -> R%d = %d\n", S, R, REG[R]);
          }
          break;

        case 0x9: /* AND */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;
            byte_t T  =  CIR[1]       & 0xF;

            REG[R] = REG[S] & REG[T];

            debug ("R%d AND R%d -> R%d = %d\n", S, T, R, REG[R]);
          }
          break;

        case 0xA: /* OR */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;
            byte_t T  =  CIR[1]       & 0xF;

            REG[R] = REG[S] | REG[T];

            debug ("R%d OR R%d -> R%d = %d\n", S, T, R, REG[R]);
          }
          break;

        case 0xB: /* XOR */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t S  = (CIR[1] >> 4) & 0xF;
            byte_t T  =  CIR[1]       & 0xF;

            REG[R] = REG[S] ^ REG[T];

            debug ("R%d XOR R%d -> R%d = %d\n", S, T, R, REG[R]);
          }
          break;

        case 0xC: /* Compare */
          {
            byte_t R  =  CIR[0]       & 0xF;
            byte_t L  = (CIR[1] >> 4) & 0xF;

            LHS = L;
            RHS = R;

            debug ("R%d <=> R%d\n", L, R);
          }
          break;

        case 0xD: /* Branch */
          {
            byte_t C  =  CIR[0] & 0xF;
            byte_t XY =  CIR[1];

            switch (C) /* Condition */
              {
              case 0x0: /* None */
                PC = XY;

                debug ("PC = %d\n", XY);
                break;

              case 0x1: /* Equal */
                if (REG[LHS] == REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d == R%d\n", LHS, RHS);
                break;

              case 0x2: /* Not Equal */
                if (REG[LHS] != REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d != R%d\n", LHS, RHS);
                break;
              
              case 0x3: /* Less Than */
                if (REG[LHS] < REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d < R%d\n", LHS, RHS);
                break;

              case 0x4: /* Greater Than */
                if (REG[LHS] > REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d > R%d\n", LHS, RHS);
                break;

              case 0x5: /* Less Than or Equal */
                if (REG[LHS] <= REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d <= R%d\n", LHS, RHS);
                break;
              
              case 0x6: /* Greater Than or Equal */
                if (REG[LHS] >= REG[RHS])
                {
                  PC = XY;

                  debug ("PC = %d\n", XY);
                }

                debug ("R%d >= R%d\n", LHS, RHS);
                break;
              
              default:
                SR = SR_FAILURE;
                break;
              }
            break;
          }

        case 0xE: /* Input */
          {
            byte_t R = CIR[0] & 0xF;

            do
            {
              printf ("%s", "IN: ");

              int val;

              if (scanf ("%d", &val) != 1)
                continue;

              if (127 < val || val < -128)
                continue;

              REG[R] = val;
              
            } while (0);

            debug ("IN R%d <- %d\n", R, REG[R]);
          }
          break;

        case 0xF: /* Output */
          {
            byte_t R = CIR[0] & 0xF;

            printf ("OUT: %d\n", REG[R]);

            debug ("OUT R%d -> %d\n", R, REG[R]);
          }
          break;
        }
    }
}

int
main (int argc, char *argv[])
{
  int optind = args (argc, argv);
  
  if (optind == -1)
    {
      error (strerror (EINVAL));
    }

  if (optind >= argc)
    {
      usage (argv);
      exit (EXIT_FAILURE);
    }

  load (argv[optind]);
  run ();
}
