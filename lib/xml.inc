# xml.inc - Babs XML routines
# $Id$

use XML::DOM;
use constant XML_STORY_LATEST	=> 1;
use constant XML_STORY_ARCHIVES	=> 2;

sub _xml_init
{
	my ($this) = @_;
	$this->{xp} = XML::DOM::Parser->new();
	return;
}

sub _xml_cleanup
{
	$this->{xp}->dispose;
}

sub xml_getfile
{
	my ($this, $type) = @_;
	$type = -1 unless defined $type;
	if ($type < 0 || $type >= @{ $this->{xml_files} })
	{
		$this->{wasp}->throw("Invalid XML file type; type: $type") 
	}
	return $this->{xml_files}[$type];
}

sub xml_update
{
	my ($this, $type, $data, $doc) = @_;
	unless (defined $doc)
	{
		my $file = $this->xml_getfile($type);
		$doc = $this->{xp}->parsefile($file);
	}
	if ($type == XML_STORY_LATEST)
	{
		# Here, we have to find the <item> corresponding
		# to our story and update its contents.
	} elsif ($type == XML_STORY_ARCHIVES) {
		
	} else {
		$this->{wasp}->throw("Unrecognized XML file type; type: $type");
	}
}

sub xml_write
{
	my ($this, $type, $doc) = @_;
	my $file = $this->xml_getfile($type);
	$doc->printToFile($file);
}

sub xml_add
{
	my ($this, $type, $data, $doc) = @_;
}

sub xml_remove
{
	my ($this, $type, $data, $doc) = @_;
}

sub xml_setup
{
	my ($this, $type) = @_;
}

return 1;
