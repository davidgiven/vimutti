# vimutti

A very unfinished and prototype tool for decrypting Buddha Machine flash
images. Based on work by Malvineous and uzlonewolf at
https://www.reddit.com/r/BigCliveDotCom/comments/pmt390/buddha_machine_teardown_with_flash_dump/.

To use, build with the Makefile. Then do:

    ./vimutti -f buddha.bin -l

...to show the directory. To extract an unencrypted file, do:

	./vimutti -f buddha.bin -x play_list.bin

(wildcards are allowed). To extract an encrypted file, do:

	./vimutti -f buddha.bin -n 16 -m 0x8880 -p 0xef -q 0x21 -x code.app

...where:

  - `-n`: size of the key
  - `-m`: reload mask
  - `-p`: initial value
  - `-q`: value to XOR with at each reload time
 
(Note that this key doesn't appear to work quite right yet!)

