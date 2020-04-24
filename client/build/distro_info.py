# Copyright (C) 2009-2012, Benjamin Drung <bdrung@debian.org>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

"""provides information about Ubuntu's and Debian's distributions"""

import csv
import datetime
import os


def convert_date(string):
    """Convert a date string in ISO 8601 into a datetime object."""
    if not string:
        date = None
    else:
        parts = [int(x) for x in string.split("-")]
        if len(parts) == 3:
            (year, month, day) = parts
            date = datetime.date(year, month, day)
        elif len(parts) == 2:
            (year, month) = parts
            if month == 12:
                date = datetime.date(year, month, 31)
            else:
                date = datetime.date(year, month + 1, 1) - datetime.timedelta(1)
        else:
            raise ValueError("Date not in ISO 8601 format.")
    return date


def _get_data_dir():
    """Get the data directory based on the module location."""
    return "/usr/share/distro-info"


class DistroDataOutdated(Exception):
    """Distribution data outdated."""

    def __init__(self):
        super(DistroDataOutdated, self).__init__(
            "Distribution data outdated. "
            "Please check for an update for distro-info-data. See "
            "/usr/share/doc/distro-info-data/README.Debian for details.")


class DistroRelease(object):
    """Represents a distributions release"""
    # pylint: disable=too-few-public-methods

    def __init__(self, version, codename, series, created=None, release=None, eol=None,
                 eol_server=None):
        # pylint: disable=too-many-arguments
        self.version = version
        self.codename = codename
        self.series = series
        self.created = created
        self.release = release
        self.eol = eol
        self.eol_server = eol_server

    def is_supported(self, date):
        """Check whether this release is supported on the given date."""
        return date >= self.created and (self.eol is None or date <= self.eol or (
            self.eol_server is not None and date <= self.eol_server))


def _get_date(row, column):
    return convert_date(row[column]) if column in row else None


class DistroInfo(object):
    """Base class for distribution information.
    Use DebianDistroInfo or UbuntuDistroInfo instead of using this directly.
    """

    def __init__(self, distro):
        self._distro = distro
        filename = os.path.join(_get_data_dir(), distro.lower() + ".csv")
        csvfile = open(filename)
        csv_reader = csv.DictReader(csvfile)
        self._releases = []
        for row in csv_reader:
            release = DistroRelease(row['version'], row['codename'], row['series'],
                                    _get_date(row, 'created'), _get_date(row, 'release'),
                                    _get_date(row, 'eol'), _get_date(row, 'eol-server'))
            self._releases.append(release)
        csvfile.close()
        self._date = datetime.date.today()

    @property
    def all(self):
        """List codenames of all known distributions."""
        return [x.series for x in self._releases]

    def get_all(self, result="codename"):
        """List all known distributions."""
        return [self._format(result, x) for x in self._releases]

    def _avail(self, date):
        """Return all distributions that were available on the given date."""
        return [x for x in self._releases if date >= x.created]

    def codename(self, release, date=None, default=None):
        """Map codename aliases to the codename they describe."""
        # pylint: disable=no-self-use,unused-argument
        return release

    def version(self, name, default=None):
        """Map codename or series to version"""
        for release in self._releases:
            if name in (release.codename, release.series):
                return release.version
        return default

    def devel(self, date=None, result="codename"):
        """Get latest development distribution based on the given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._avail(date) if x.release is None or
                   (date < x.release and (x.eol is None or date <= x.eol))]
        if not distros:
            raise DistroDataOutdated()
        return self._format(result, distros[-1])

    def _format(self, format_string, release):
        """Format a given distribution entry."""
        if format_string == "object":
            return release
        if format_string == "codename":
            return release.series
        if format_string == "fullname":
            return self._distro + " " + release.version + ' "' + release.codename + '"'
        if format_string == "release":
            return release.version

        raise ValueError("Only codename, fullname, object, and release are allowed "
                         "result values, but not '" + format_string + "'.")

    def stable(self, date=None, result="codename"):
        """Get latest stable distribution based on the given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._avail(date) if x.release is not None and
                   date >= x.release and (x.eol is None or date <= x.eol)]
        if not distros:
            raise DistroDataOutdated()
        return self._format(result, distros[-1])

    def supported(self, date=None, result=None):
        """Get list of all supported distributions based on the given date."""
        raise NotImplementedError()

    def valid(self, codename):
        """Check if the given codename is known."""
        return codename in self.all

    def unsupported(self, date=None, result="codename"):
        """Get list of all unsupported distributions based on the given date."""
        if date is None:
            date = self._date
        supported = self.supported(date)
        distros = [self._format(result, x) for x in self._avail(date)
                   if x.series not in supported]
        return distros


class DebianDistroInfo(DistroInfo):
    """provides information about Debian's distributions"""

    def __init__(self):
        super(DebianDistroInfo, self).__init__("Debian")

    def codename(self, release, date=None, default=None):
        """Map 'unstable', 'testing', etc. to their codenames."""
        if release == "unstable":
            codename = self.devel(date)
        elif release == "testing":
            codename = self.testing(date)
        elif release == "stable":
            codename = self.stable(date)
        elif release == "oldstable":
            codename = self.old(date)
        else:
            codename = default
        return codename

    def devel(self, date=None, result="codename"):
        """Get latest development distribution based on the given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._avail(date) if x.release is None or
                   (date < x.release and (x.eol is None or date <= x.eol))]
        if len(distros) < 2:
            raise DistroDataOutdated()
        return self._format(result, distros[-2])

    def old(self, date=None, result="codename"):
        """Get old (stable) Debian distribution based on the given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._avail(date)
                   if x.release is not None and date >= x.release]
        if len(distros) < 2:
            raise DistroDataOutdated()
        return self._format(result, distros[-2])

    def supported(self, date=None, result="codename"):
        """Get list of all supported Debian distributions based on the given
           date."""
        if date is None:
            date = self._date
        distros = [self._format(result, x) for x in self._avail(date)
                   if x.eol is None or date <= x.eol]
        return distros

    def testing(self, date=None, result="codename"):
        """Get latest testing Debian distribution based on the given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._avail(date) if (x.release is None and x.version) or
                   (x.release is not None and date < x.release and
                    (x.eol is None or date <= x.eol))]
        if not distros:
            raise DistroDataOutdated()
        return self._format(result, distros[-1])

    def valid(self, codename):
        """Check if the given codename is known."""
        return (DistroInfo.valid(self, codename) or
                codename in ["unstable", "testing", "stable", "oldstable"])


class UbuntuDistroInfo(DistroInfo):
    """provides information about Ubuntu's distributions"""

    def __init__(self):
        super(UbuntuDistroInfo, self).__init__("Ubuntu")

    def lts(self, date=None, result="codename"):
        """Get latest long term support (LTS) Ubuntu distribution based on the
           given date."""
        if date is None:
            date = self._date
        distros = [x for x in self._releases if x.version.find("LTS") >= 0 and
                   x.release <= date <= x.eol]
        if not distros:
            raise DistroDataOutdated()
        return self._format(result, distros[-1])

    def is_lts(self, codename):
        """Is codename an LTS release?"""
        distros = [x for x in self._releases if x.series == codename]
        if not distros:
            return False
        return "LTS" in distros[0].version

    def supported(self, date=None, result="codename"):
        """Get list of all supported Ubuntu distributions based on the given
           date."""
        if date is None:
            date = self._date
        distros = [self._format(result, x) for x in self._avail(date)
                   if date <= x.eol or
                   (x.eol_server is not None and date <= x.eol_server)]
        return distros
