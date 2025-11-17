require 'time'
require 'yaml'

$stdout.sync = true

def parseTime(time)
  # input e.g.: 2022-12-24 11:59:59 +0100
  DateTime.parse(time).new_offset(0).strftime("%Y-%m-%d %H:%M:%S")
end

data = {}
file_dir = File.dirname(__FILE__)
Dir.chdir(file_dir + "/..") do
  time_sha1 = `git log -n 1 --pretty="format:%cd|%h" --date=iso -- . 2>&1`.split("|")
  time = parseTime(time_sha1[0])
  sha1 = time_sha1[1]

  data["."] = "Based on Eclipse OpenBSW, from #{time}, commit #{sha1}."
end

File.write(file_dir + "/footer.yaml", data.to_yaml)
