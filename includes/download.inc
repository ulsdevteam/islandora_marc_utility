<?php

/**
 * @file
 * Form for downloading an individual MARC or MODS xml file that comes from a
 * MRC or MARCXML collection.
 */

/**
 * Form building function.
 */
function islandora_marc_utility_download($form, &$form_state, $hash) {
  $joined = base64_decode($hash);
  @list($filename, $bib_id, $subject, $fedora_PID) = explode("|", $joined);

  if (file_exists($filename) && $bib_id && $subject) {
    // Set a value for display purposes.
    $bib_id = ($bib_id) ? $bib_id : 'unknown identifier';

    $download_filename = $bib_id . '_' . $subject . '.xml';

    header('Content-Type: text/xml');
    header('Content-disposition: attachment; filename='.$download_filename);
    header('Content-Transfer-Encoding: Binary');
    header('Content-Length: ' . filesize($filename));
    readfile($filename);
    exit;
  }
  else {
    drupal_set_message(t('The underlying file could not be found (or the ' .
        'permissions were not sufficient to access) in order to serve it up ' .
        'for download.'));
  }
}
