<?php

/**
 * @file
 * Form for viewing an individual MARC or MODS that comes out of a MRC or
 * MARCXML collection.
 */

/**
 * Form building function.
 */
function islandora_marc_utility_file_viewer_display($form, &$form_state, $hash) {
  $joined = base64_decode($hash);
  @list($filename, $bib_id, $subject, $fedora_PID) = explode("|", $joined);
  $file_contents = file_exists($filename) ? htmlspecialchars(file_get_contents($filename)) :
      '<i>File did not exist or the permissions were not sufficient to access ' .
      'that file\'s path.</i>';
  // Set a value for display purposes.
  $bib_id = ($bib_id) ? $bib_id : 'unknown identifier';
  $form = array(
    'foo' => array(
      '#type' => 'item',
      '#markup' => theme('islandora_marc_utility_file_viewer', array(
        'hash' => $hash,
        'file_contents' => $file_contents,
        'filename' => 'For "' . $bib_id . '", viewing ' . $subject . ' XML ' . $filename . ':',
        'subject' => $subject,
      ))
    ),
  );
  return $form;
}
