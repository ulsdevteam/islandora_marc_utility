<?php

/**
 * @file
 * islandora-marc-utility-file-viewer.tpl display template.
 s*
 * Variables available:
 * - $filename
 * - $file_contents
 */
?>
<?php print (user_access(ISLANDORA_MARC_UTILITY_MANAGE) ? l('Configure Islandora MARC Utility', 'admin/islandora/islandora_marc_utility', array('attributes' => array('target' => '_blank'))) . '<br>' : ''); ?>
<p><a href="/admin/islandora/tools/islandora_marc_utility/download/<?php print $hash; ?>">Download <?php print $subject; ?> XML</a>
<?php print ' | ' . (user_access(ISLANDORA_MARC_UTILITY_CAN_APPLY_DATASTREAM) ? '<a href="/admin/islandora/tools/islandora_marc_utility/apply_datastream/' . $hash . '">Apply datastream</a>' : ''); ?>
</p>
<b><?php print $filename; ?></b><br>
<textarea class="imu_file_viewer" readonly><?php print $file_contents; ?>
</textarea>