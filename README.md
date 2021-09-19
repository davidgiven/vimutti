# vimutti

A very unfinished and prototype tool for decrypting Buddha Machine flash
images. Based on work by Malvineous and uzlonewolf at
https://www.reddit.com/r/BigCliveDotCom/comments/pmt390/buddha_machine_teardown_with_flash_dump/.

To use, build with the Makefile. Then do:

    ./vimutti unpackimg -f buddha.bin -l

...to show the directory. To extract an unencrypted file, do:

	./vimutti unpackimg -f buddha.bin -x play_list.bin

(wildcards are allowed). Once you've extracted the code.app, you can use the
same syntax with `unpackapp` to extract a code chunk from it. (Use the chunk
index rather than a filename.)

