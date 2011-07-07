# -*- ruby -*-

require 'rubygems'
require 'hoe'
require 'rake/extensiontask'

# Hoe.plugin :compiler
# Hoe.plugin :gem_prelude_sucks
# Hoe.plugin :inline
# Hoe.plugin :racc
# Hoe.plugin :rubyforge

Hoe.spec 'skypekit' do
  developer('Alexey Vasiliev', 'leopard.not.a@gmail.com')
  self.readme_file   = 'README.rdoc'
  self.history_file  = 'CHANGELOG.rdoc'
  self.extra_rdoc_files  = FileList['*.rdoc']
  self.extra_dev_deps << ['rake-compiler', '>= 0']
  self.spec_extras = { :extensions => ["ext/skypekit/extconf.rb"] }

  Rake::ExtensionTask.new('skypekit', spec) do |ext|
    ext.lib_dir = File.join('lib', 'skypekit')
    ext.source_pattern = "*.{c,cpp}"
    #ext.config_options << '--with-skypekit-dir'
  end

  # self.rubyforge_name = 'skypekitx' # if different than 'skypekit'
end

Rake::Task[:test].prerequisites << :compile

# vim: syntax=ruby
