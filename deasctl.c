#include <stdio.h>
#include <stdlib.h>
#ifdef unix
#   include <unistd.h>
#endif
#include "deas.h"
#include "pgplib.h"

#define countbytes(r) ((countbits(r)+7)>>3)

char pn[KEYSZ], pe[KEYSZ];
char sn[KEYSZ], se[KEYSZ];
char sd[KEYSZ], sp[KEYSZ];
char sq[KEYSZ], su[KEYSZ];

char enc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@%0123456789";

void printkey (char *name, char *val)
{
	int len, i;
	int reg, n;

	for (len=KEYSZ; len>0; --len)
		if (val[len-1])
			break;
	printf ("%s ", name);

	reg = len;
	n = 8;
	i = 0;
	for (;;) {
		putchar (enc[reg & 077]);
		reg >>= 6;
		n -= 6;
		if (n < 6) {
			if (i >= len)
				break;
			reg |= (unsigned char) val[i++] << n;
			n += 8;
		}
	}
	if (n > 0)
		putchar (enc[reg & 077]);
	printf ("\n");
}

void encode ()
{
	char inbuf[KEYSZ], outbuf[KEYSZ];
	int q, n, err;

	q = countbytes ((void*)pn);
	for (;;) {
		n = read (0, inbuf, q - 4);
		if (n < 0)
			perror ("input");
		if (n <= 0)
			break;
		err = rsa_public_encrypt (outbuf, inbuf, n, pe, pn);
		if (err) {
			fprintf (stderr, "rsa_public_encrypt error %d\n", err);
			break;
		}
		if (write (1, outbuf, q) != q) {
			perror ("output");
			break;
		}
	}
}

void decode ()
{
	char inbuf[KEYSZ], outbuf[KEYSZ];
	int q, n;

	q = countbytes ((void*)sn);
	for (;;) {
		n = read (0, inbuf, q);
		if (n == 0)
			break;
		if (n != q) {
			perror ("input");
			break;
		}
		n = rsa_private_decrypt (outbuf, inbuf, se, sd, sp, sq, su, sn);
		if (n <= 0) {
			fprintf (stderr, "rsa_private_decrypt error %d\n", n);
			break;
		}
		if (write (1, outbuf, n) != n) {
			perror ("output");
			break;
		}
	}
}

void cryptfile (char *key, int decodeflag)
{
	char inbuf [512], outbuf [512];
	int q, n;

	q = 512;
	for (;;) {
		n = read (0, inbuf, q);
		if (n < 0)
			perror ("input");
		if (n <= 0)
			break;
		if (decodeflag)
			block_decrypt (inbuf, outbuf, n, key);
		else
			block_encrypt (inbuf, outbuf, n, key);
		if (write (1, outbuf, n) != n) {
			perror ("output");
			break;
		}
	}
}

#ifndef unix
char *getpass (char *prompt)
{
	static char buf[80];
	char *p = buf;

	cputs (prompt);
	for (;;) {
		*p = getch ();
		switch (*p) {
		case 0:
			continue;
		case '\b':
			if (p > buf) {
				--p;
				putch ('\b');
				putch (' ');
				putch ('\b');
			}
			continue;
		case '\n':
		case '\r':
ret:                	*p = 0;
			putch ('\r');
			putch ('\n');
			return buf;
		default:
			++p;
			putch ('*');
			if (p >= buf + sizeof(buf) - 1)
				goto ret;
		}
	}
}
#endif

int main (int argc, char **argv)
{
	char userid[KEYSZ], passwd[16], *id;
	int err, showkeyflag = 0, showsecflag = 0, decodeflag = 0;
	int encodeflag = 0, encryptflag = 0, decryptflag = 0;

	if (argc < 2 || argv[1][0] != '-')
		goto usage;
	for (--argc, ++argv; argc>0 && argv[0][0]=='-'; --argc, ++argv)
		switch (argv[0][1]) {
		case 'K':
			++showsecflag;
			showkeyflag = 0;
			continue;
		case 'k':
			++showkeyflag;
			showsecflag = 0;
			continue;
		case 'e':
			++encryptflag;
			showkeyflag = 0;
			showsecflag = 0;
			continue;
		case 'd':
			++decryptflag;
			showkeyflag = 0;
			showsecflag = 0;
			continue;
		case 'E':
			++encodeflag;
			showkeyflag = 0;
			showsecflag = 0;
			continue;
		case 'D':
			++decodeflag;
			showkeyflag = 0;
			showsecflag = 0;
			continue;
		default:
usage:                  fprintf (stderr, "Usage:\n");
			fprintf (stderr, "\tdeasctl -e password < infile > outfile\n");
			fprintf (stderr, "\t\t- encrypt file with password\n");
			fprintf (stderr, "\tdeasctl -d password < infile > outfile\n");
			fprintf (stderr, "\t\t- decrypt file with password\n");
			fprintf (stderr, "\tdeasctl -E user < infile > outfile\n");
			fprintf (stderr, "\t\t- encrypt file using public key\n");
			fprintf (stderr, "\tdeasctl -D user < infile > outfile\n");
			fprintf (stderr, "\t\t- decrypt file using private key\n");
			fprintf (stderr, "\tdeasctl -k user\n");
			fprintf (stderr, "\t\t- show user public key\n");
			fprintf (stderr, "\tdeasctl -K user\n");
			fprintf (stderr, "\t\t- show user secret key\n");
			return 1;
		}

	if (argc != 1)
		goto usage;
	if (encryptflag || decryptflag) {
		cryptfile (argv[0], decryptflag);
		return 0;
	}

	id = argv[0];
#ifdef unix
	{
	static char filename[128], *p;
	p = getenv ("HOME");
	if (! p)
		p = "/";
	strcpy (filename, p);
	strcat (filename, "/.deasdat");
	crypt_pubfile = crypt_secfile = filename;
	}
#else
	crypt_pubfile = crypt_secfile = "deas.dat";
#endif

	if (! decodeflag && ! showsecflag) {
		strcpy (userid, id);
		err = getpublickey (crypt_pubfile, 0, userid, pn, pe);
		if (err < 0) {
			fprintf (stderr, "getpublickey error %d\n", err);
			exit (1);
		}
	}
	if (showkeyflag) {
		fprintf (stderr, "Public key: %.*s\n", userid[0], userid+1);
		printkey ("keye", pe);
		printkey ("keyn", pn);
	}
	if (showsecflag || decodeflag) {
		/* Get password. */
		char mdContext[100];
		char *pass = getpass ("Password: ");

		/* Calculate the hash */
		MD5Init(mdContext);
		MD5Update(mdContext, pass, strlen (pass));
		memset(pass, 0, strlen (pass));
		MD5Final(passwd, mdContext);

		strcpy (userid, id);
		err = getsecretkey (passwd, crypt_secfile, userid, sn, se, sd, sp, sq, su);
		if (err < 0) {
			fprintf (stderr, "getsecretkey error %d\n", err);
			exit (1);
		}
	}

	if (showkeyflag)
		;
	else if (showsecflag) {
		fprintf (stderr, "Secret key: %.*s\n", userid[0], userid+1);
		printkey ("syse", se);
		printkey ("sysn", sn);
		printkey ("sysd", sd);
		printkey ("sysp", sp);
		printkey ("sysq", sq);
		printkey ("sysu", su);
	} else if (decodeflag)
		decode ();
	else
		encode ();
	return 0;
}
