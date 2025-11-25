# DNS Toggle

![dns toggle logo](https://cdn.ittykeys.com/dns-toggle.png)
![dns toggle screenshot](https://cdn.ittykeys.com/dns-toggle-scr.png)

## Description

Simple GUI toggle for /etc/resolv.conf for us old dogs still handling DNS that way.

### Features

* Toggling between 2 main DNS servers
* Toggle for a 3rd DNS server that overrides the 2 main (useful for VPN etc)
* Handles empty /etc/resolv.conf and creates it when we toggle
* Should leave alone things like domain, search and comments

### Dependencies

* libgtk-3-dev
* librsvg2-dev

### Build

Make file **nameservers.conf** before building, example:
```
nameserver 1.1.1.1
nameserver 8.8.8.8
nameserver 9.9.9.9
```

Then:


```bash
sudo apt install libgtk-3-dev librsvg2-dev build-essential
git clone git@github.com:ittykeys/dns-toggle.git
cd dns-toggle
make
```

### Run

Please backup your precious /etc/resolv.conf before running:

```bash
sudo cp /etc/resolv.conf /etc/resolv.conf.bk
```

Then:

```bash
./dns-toggle
```
It will ask for authentication via **pkexec** when toggling.

##### Todo

* Detect and re-toggle previous state (and toggle state) properly after 3rd dns toggled (especially at startup).
* Detect and error out if /etc/resolv.conf is handled by another system.
* Future-proof for gtk4
* Add flag to auth immediately at launch, negating the need for auth every toggle
* Windows version (are network connection options exposed via win api?)
* Pack assets in executable
* Don't block main gui thread for auth (#3500)
* KDE widget (#3480)

### License

This project is licensed under the GNU General Public License - see the [License file](LICENSE) file for details.