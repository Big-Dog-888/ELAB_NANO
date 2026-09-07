# =========================================================================
#   CMock - Automatic Mock Generation for C
#   ThrowTheSwitch.org
#   Copyright (c) 2007-26 Mike Karlesky, Mark VanderVoord, & Greg Williams
#   SPDX-License-Identifier: MIT
# =========================================================================

require 'yaml'
require 'fileutils'
require '../cmock/vendor/unity/auto/unity_test_summary'
require '../cmock/vendor/unity/auto/generate_test_runner'
require '../cmock/vendor/unity/auto/colour_reporter'

module RakefileHelpers
  $return_error_on_failures = false

  C_EXTENSION = '.c'.freeze

  def load_yaml(yaml_string)
    YAML.load(yaml_string, aliases: true)
  rescue ArgumentError
    YAML.load(yaml_string)
  end

  def find_cmock_target(targets_dir, config_file)
    return config_file if File.exist?("#{targets_dir}/#{config_file}")

    basename = File.basename(config_file, '.yml')
    while basename.include?('_')
      basename = basename.rpartition('_').first
      candidate = "#{basename}.yml"
      return candidate if File.exist?("#{targets_dir}/#{candidate}")
    end

    nil
  end

  def load_configuration(config_file, cmock_overlay = nil)
    $cfg_file = config_file
    $proj = load_yaml(File.read('./project.yml'))

    unity_targets_dir = '../cmock/vendor/unity/test/targets'
    cmock_targets_dir = '../cmock/test/targets'
    config_basename   = File.basename(config_file)
    path_specified    = File.dirname(config_file) != '.'

    # Resolve the target file location:
    #   - path specified → use only that location
    #   - no path → check current directory first, then vendor unity targets
    unity_target = if path_specified
                     config_file
                   elsif File.exist?("./#{config_file}")
                     "./#{config_file}"
                   else
                     "#{unity_targets_dir}/#{config_file}"
                   end

    if File.exist?(unity_target)
      puts "Loading Unity target:  #{unity_target}"
      $unity_cfg = load_yaml(File.read(unity_target))

      cmock_file = cmock_overlay || find_cmock_target(cmock_targets_dir, config_basename)
      if cmock_file
        puts "Loading CMock overlay: #{cmock_targets_dir}/#{cmock_file}"
        $cmock_cfg = load_yaml(File.read("#{cmock_targets_dir}/#{cmock_file}"))
      else
        puts "No CMock overlay found for #{config_file}"
        $cmock_cfg = {}
      end
    else
      # CMock-only target (no Unity equivalent); it uses Unity format directly
      puts "Loading CMock-only target: #{cmock_targets_dir}/#{config_basename}"
      $unity_cfg = load_yaml(File.read("#{cmock_targets_dir}/#{config_basename}"))
      $cmock_cfg = {}
    end

    $colour_output = $proj[:project][:colour]
  end

  def configure_clean
    CLEAN.include("#{$proj[:project][:build_root]}*.*")
  end

  def configure_toolchain(config_file = DEFAULT_CONFIG_FILE, cmock_overlay = nil)
    config_file ||= DEFAULT_CONFIG_FILE
    config_file += '.yml' unless config_file =~ /\.yml$/i
    cmock_overlay += '.yml' if cmock_overlay && cmock_overlay !~ /\.yml$/i
    load_configuration(config_file, cmock_overlay)
    configure_clean
  end

  def unit_test_files
    base = $proj[:paths][:test].to_s
    patterns = [
      base + "Test*#{C_EXTENSION}",
      base + "test_*#{C_EXTENSION}",
      base + "*/*test*#{C_EXTENSION}",
      base + "*/*Test*#{C_EXTENSION}",
    ]
    result = FileList.new
    patterns.each { |p| result.include(p.tr('\\', '/')) }
    result.exclude(/Runner/)
    result.exclude(/^build\//)
    result.exclude('/build/')
    result
  end

  def local_include_dirs
    $proj[:paths][:include].reject { |dir| dir.is_a?(Array) }
  end

  def extract_headers(filename)
    includes = []
    lines = File.readlines(filename)
    lines.each do |line|
      m = line.match(/^\s*#include\s+"\s*(.+\.[hH])\s*"/)
      includes << m[1] unless m.nil?
    end
    includes
  end

  def find_source_file(header, paths)
    paths.each do |dir|
      src_file = dir + header.ext(C_EXTENSION)
      return src_file if File.exist?(src_file)
    end
    nil
  end

  def tackit(strings)
    case strings
    when Array
      "\"#{strings.join}\""
    when /^-/
      strings
    when /\s/
      "\"#{strings}\""
    else
      strings
    end
  end

  # All defines: project common + Unity target + CMock overlay + any extras
  def all_defines(extra = [])
    (($proj[:defines][:test] || []) +
     ($unity_cfg[:defines][:test] || []) +
     (($cmock_cfg[:defines] || {})[:test] || []) +
     extra).uniq
  end

  # Toolchain-specific include paths: Array items in Unity's :paths: :test:
  def toolchain_include_paths
    if $unity_cfg[:paths] && $unity_cfg[:paths][:test]
      $unity_cfg[:paths][:test]
    else
      []
    end
  end

  # Resolve argument template tokens into a flat argument string.
  # Supports Ceedling-style positional tokens and legacy Unity COLLECTION_* tokens.
  #   ${5}  → expands to one arg per include path (toolchain paths + project paths combined)
  #   ${6}  → expands to one arg per define
  #   ${1}  → input file(s)
  #   ${2}  → output file
  def build_argument_list(raw_args, toolchain_paths, project_paths, defines, input, output)
    result = []
    raw_args.each do |arg|
      if arg.is_a?(Array)
        result << arg.join
      elsif arg.include?('${5}')
        (toolchain_paths + project_paths).each do |p|
          result << arg.gsub('${5}', p.is_a?(Array) ? p.join : p.to_s)
        end
      elsif arg.include?('${6}')
        defines.each { |d| result << arg.gsub('${6}', d) }
      elsif arg.include?('COLLECTION_PATHS_TEST_TOOLCHAIN_INCLUDE')
        toolchain_paths.each { |p| result << "-I\"#{p.is_a?(Array) ? p.join : p}\"" }
      elsif arg.include?('COLLECTION_PATHS_TEST_SUPPORT_SOURCE_INCLUDE_VENDOR')
        project_paths.each { |p| result << "-I\"#{p}\"" }
      elsif arg.include?('COLLECTION_DEFINES_TEST_AND_VENDOR')
        defines.each { |d| result << "-D#{d}" }
      else
        result << arg.gsub('${1}', input.to_s).gsub('${2}', output.to_s)
      end
    end
    result.join(' ')
  end

  def compile(file, extra_defines = [])
    tool       = $unity_cfg[:tools][:test_compiler]
    ext        = $unity_cfg[:extension][:object] || '.o'
    build_root = $proj[:project][:build_root] || 'build/'
    obj_file   = build_root + File.basename(file, C_EXTENSION) + ext

    cmd_str = "#{tackit(tool[:executable])} #{
              build_argument_list(tool[:arguments],
                                  toolchain_include_paths,
                                  $proj[:paths][:include],
                                  all_defines(extra_defines),
                                  file, obj_file)}"
    execute(cmd_str)
    File.basename(obj_file)
  end

  def link_it(exe_name, obj_list)
    tool       = $unity_cfg[:tools][:test_linker]
    ext        = $unity_cfg[:extension][:executable] || ''
    build_root = $proj[:project][:build_root] || 'build/'

    input_files = obj_list.uniq.map { |obj| build_root + obj }.join(' ')
    output_file = build_root + exe_name + ext

    cmd_str = "#{tackit(tool[:executable])} #{build_argument_list(tool[:arguments], [], [], [], input_files, output_file)}"
    execute(cmd_str)
  end

  def build_simulator_fields
    return nil unless $unity_cfg[:tools][:test_fixture]

    tool       = $unity_cfg[:tools][:test_fixture]
    executable = tackit(tool[:executable])
    raw_args   = tool[:arguments] || []
    idx        = raw_args.index('${1}')
    if idx
      pre  = raw_args[0...idx].map { |a| a.is_a?(Array) ? a.join : a }.join(' ')
      post = raw_args[(idx + 1)..].map { |a| a.is_a?(Array) ? a.join : a }.join(' ')
    else
      pre  = ''
      post = raw_args.map { |a| a.is_a?(Array) ? a.join : a }.join(' ')
    end
    { command: "#{executable} ", pre_support: pre, post_support: post }
  end

  def execute(command_string, verbose = true, ok_to_fail = false)
    report command_string
    output = `#{command_string}`.chomp
    report(output) if verbose && !output.nil? && !output.empty?
    unless (!$?.nil? && $?.exitstatus.zero?) || ok_to_fail
      raise "Command failed. (Returned #{$?.exitstatus})"
    end

    output
  end

  def report_summary
    summary = UnityTestSummary.new
    summary.root = HERE
    results_glob = "#{$proj[:project][:build_root]}*.test*"
    results_glob.tr!('\\', '/')
    results = Dir[results_glob]
    summary.targets = results
    report summary.run
    raise 'There were failures' if (summary.failures > 0) && $return_error_on_failures
  end

  def run_tests(test_files)
    report 'Running system tests...'

    load_configuration($cfg_file)

    include_dirs = local_include_dirs

    source_files_from_config = collect_sources_from_paths

    test_files.each do |test|
      direct_headers = extract_headers(test) + ['cmock.h'] + [($proj[:cmock] || {})[:unity_helper_path]]
      direct_headers.compact!

      mock_headers = direct_headers.select { |h| h =~ /Mock/ }
      mock_headers.each do |header|
        require '../cmock/lib/cmock'
        @cmock ||= CMock.new($proj[:cmock])
        real_name = header.gsub(/^Mock/, '').gsub(/_Mock/, '')
        found = resolve_header(real_name, local_include_dirs)
        if found.nil?
          ($proj[:paths][:source] || []).each do |dir|
            candidate = File.expand_path(dir + real_name)
            found = candidate if File.exist?(candidate)
          end
        end
        @cmock.setup_mocks([found].compact) if found
      end

      all_headers = collect_all_headers(direct_headers, include_dirs)
      all_sources = collect_all_sources(all_headers, include_dirs)
      all_sources += collect_all_sources_from_sources(all_sources, include_dirs)
      all_sources += source_files_from_config

      obj_list = []
      compiled_sources = {}

      mock_src_dir = $proj[:cmock][:mock_path] rescue nil
      mock_src_dir ||= 'build/mocks'
      Dir.glob("#{mock_src_dir}/Mock*.c").each do |mock_c|
        next unless File.exist?(mock_c)
        key = File.expand_path(mock_c)
        next if compiled_sources[key]
        obj_list << compile(mock_c, ['TEST'])
        compiled_sources[key] = true
      end

      all_sources.each do |src_file|
        next unless File.exist?(src_file)
        key = File.expand_path(src_file)
        next if compiled_sources[key]
        obj_list << compile(src_file, ['TEST'])
        compiled_sources[key] = true
      end

      test_base   = File.basename(test, C_EXTENSION)
      runner_name = "#{test_base}_Runner.c"
      runner_path = "#{$proj[:project][:build_root]}#{runner_name}"
      UnityTestRunnerGenerator.new({}).run(test, runner_path)

      obj_list << compile(runner_path, ['TEST'])
      obj_list << compile(test, ['TEST'])

      link_it(test_base, obj_list)

      simulator  = build_simulator_fields
      build_root = $proj[:project][:build_root]
      executable = build_root + test_base + ($unity_cfg[:extension][:executable] || '')
      cmd_str = if simulator.nil?
                  executable
                else
                  "#{simulator[:command]} #{simulator[:pre_support]} #{executable} #{simulator[:post_support]}"
                end
      output = execute(cmd_str, true)
      test_results = build_root + test_base
      stripped = output.gsub(/\e\[[0-9;]*m/, '')
      test_results += stripped.match(/OK\s*$/).nil? ? '.testfail' : '.testpass'
      File.open(test_results, 'w') { |f| f.print output }
    end
  end

  def collect_sources_from_paths
    sources = []
    ($proj[:paths][:source] || []).each do |dir|
      Dir.glob(File.join(dir, '**', '*.c')).each do |f|
        sources << f.tr('\\', '/')
      end
    end
    sources
  end

  def collect_all_headers(initial_headers, include_dirs, visited = {})
    result = []
    queue = initial_headers.dup
    while (header = queue.shift)
      next if visited[header]
      visited[header] = true
      result << header
      full_path = resolve_header(header, include_dirs)
      next if full_path.nil?
      transitive = extract_headers(full_path)
      transitive.each do |h|
        queue << h unless visited[h]
      end
    end
    result
  end

  def collect_all_sources(headers, include_dirs)
    sources = []
    headers.each do |header|
      src = find_source_file(header, include_dirs)
      sources << src unless src.nil?
    end
    sources
  end

  def collect_all_sources_from_sources(sources, include_dirs, visited = {})
    result = []
    queue = sources.dup
    while (src = queue.shift)
      key = File.expand_path(src)
      next if visited[key]
      visited[key] = true
      result << src
      src_dir = File.dirname(src) + '/'
      local_includes = extract_headers(src)
      local_includes.each do |inc|
        candidate = File.expand_path(inc, src_dir)
        if File.exist?(candidate)
          candidate_src = candidate.sub(/\.h$/, C_EXTENSION)
          queue << candidate_src if File.exist?(candidate_src)
        end
        resolved = resolve_header(inc, include_dirs)
        unless resolved.nil?
          inc_src = resolved.sub(/\.h$/, C_EXTENSION)
          queue << inc_src if File.exist?(inc_src)
        end
      end
    end
    result
  end

  def resolve_header(header, include_dirs)
    include_dirs.each do |dir|
      candidate = File.expand_path(dir + header)
      return candidate if File.exist?(candidate)
    end
    nil
  end

  def build_application(main)
    report 'Building application...'

    obj_list = []
    load_configuration($cfg_file)
    main_path = $proj[:paths][:source].first + main + C_EXTENSION

    # Detect dependencies and build required modules
    include_dirs = local_include_dirs
    extract_headers(main_path).each do |header|
      src_file = find_source_file(header, include_dirs)
      obj_list << compile(src_file) unless src_file.nil?
    end

    # Build the main source file
    obj_list << compile(main_path)

    # Create the executable
    link_it(File.basename(main_path, C_EXTENSION), obj_list)
  end
end