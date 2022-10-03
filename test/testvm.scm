(use-modules (gnu) (guix))
(use-service-modules base)
(use-package-modules admin certs linux nvi suckless)

(define-public %testvm-services
  (list
   (service login-service-type)
   (service virtual-terminal-service-type)
   (syslog-service)
   (service agetty-service-type (agetty-configuration
                                 (extra-options '("-L")) ; no carrier detect
                                 (term "vt100")
                                 (tty #f) ; automatic
                                 (shepherd-requirement '(syslogd))))

   (service mingetty-service-type (mingetty-configuration
                                   (tty "tty1")))
   (service static-networking-service-type
            (list %loopback-static-networking))
   (service udev-service-type)
   (service special-files-service-type
            `(("/bin/sh" ,(file-append sbase "/bin/sh"))
              ("/usr/bin/env" ,(file-append sbase "/bin/env"))))))

(define-public %testvm-packages
  (list
   kmod sbase iproute isc-dhcp nvi fbset))

(define-public testvm-os
  (operating-system
    (host-name "testvm")
    (timezone "Etc/UTC")
    (locale "en_US.utf8")

    (firmware '())
    (skeletons '())

    (bootloader (bootloader-configuration
                 (bootloader grub-bootloader)
                 (targets '("/dev/nop"))
                 (terminal-outputs '(console))))
    (file-systems (cons (file-system
                          (mount-point "/")
                          (device "/dev/nop")
                          (type "ext4"))
                        %base-file-systems))
    (packages %testvm-packages)
    (services %testvm-services)))

testvm-os
