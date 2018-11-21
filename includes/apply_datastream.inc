<?php

/**
 * @file
 * Form for applying the datastream for an individual MARC or MODS record that
 * comes from a MRC or MARCXML collection.
 */

/**
 * Form building function.
 */
function islandora_marc_utility_apply_datastream_handler($form, &$form_state, $hash) {
  $joined = base64_decode($hash);
  @list($filename, $bib_id, $subject, $fedora_PID) = explode("|", $joined);
  $file_contents = file_exists($filename) ? htmlspecialchars(file_get_contents($filename)) :
      '<i>File did not exist or the permissions were not sufficient to access ' .
      'that file\'s path.</i>';
  // Set a value for display purposes.
  $bib_id = ($bib_id) ? $bib_id : 'unknown identifier';
  $islandora_object = islandora_object_load($fedora_PID);
  $form = array(
    'foo' => array(
      '#type' => 'item',
      '#markup' => theme('islandora_marc_utility_apply_datastream', array(
        'file_contents' => $file_contents,
        'filename' => 'For "' . $bib_id . '", viewing ' . $subject . ' file: ' . $filename . '.',
        'subject' => $subject,
        'has_object' => (is_object($islandora_object)),
        'Fedora_PID' => (is_object($islandora_object) ?
          l($fedora_PID, 'islandora/object/' . $fedora_PID) :
          $fedora_PID),
      ))
    ),
  );
  return $form;
}
