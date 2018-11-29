# Islandora MRC Import [![Build Status](https://travis-ci.org/ulsdevteam/islandora_marc_utility.png?branch=7.x)](https://travis-ci.org/ulsdevteam/islandora_marc_utility)

## Introduction

This module provides a method to import MARC mrc records.

Upload either a MARC mrc file or a MARCXML (collection) file and be able to:
1. Download a ZIP file of the MARC and MODS
2. Apply the MARC and MODS to existing objects.
3. Access these functions from other modules.

## Requirements

This module requires PHP version 5.5 at least.  This module also requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Islandora Solr Search](https://github.com/Islandora/islandora_solr_search) - Searches an Islandora Solr index.  This is required in order to find matching objects that have a specific MODS value.

**Required software**
For some of the operations of this module, some other software should be installed on the web server that will be running this.
`File_MARC` is required in order that MARC mrc files can be parsed.  To install File_MARC from the command line, simply:
```
$ pear install File_MARC-1.3.0
```
*For info on the File_MARC library, see [https://pear.php.net/manual/en/package.fileformats.file-marc.reading.php](https://pear.php.net/manual/en/package.fileformats.file-marc.reading.php).*


# Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

N/A

## Documentation

Further documentation for this module is available at [our wiki](https://wiki.duraspace.org/display/ISLANDORA/How+to+Batch+Ingest+Files).

### Usage

*Pending development.*

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Author / License

Written by [Willow Gillingham](https://github.com/bgilling) for the [University of Pittsburgh](http://www.pitt.edu).  Copyright (c) University of Pittsburgh.

Released under a license of GPL v2 or later.
