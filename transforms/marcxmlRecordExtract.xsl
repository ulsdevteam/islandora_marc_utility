<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:marc="http://www.loc.gov/MARC21/slim">
    <xsl:output method="xml" indent="yes" encoding="utf-8"/>
    <xsl:template match="/">
        <xsl:for-each select="/marc:collection/marc:record">
            <xsl:copy-of select="." />
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>
