Pod::Spec.new do |spec|
  spec.name         = 'TCGSDK'
  spec.version      = '1.3.2'
  spec.license      = { :type => 'MIT' }
  spec.homepage     = 'https://github.com/xxhapecheng/cloudgame-ios-sdk'
  spec.summary      = 'Tencent Cloud Gaming Software Development Kit for iOS.'
  spec.authors      = { 'xxhapecheng' => 'xxhapecheng@tencent.com' }
  spec.source       = { :path => './' }
  spec.pod_target_xcconfig = {
    'ENABLE_BITCODE' => 'OFF'
  }
  spec.vendored_frameworks = 'SDK/TCGSDK.framework', 'SDK/TWEBRTC.framework'
  spec.info_plist = {
    'UISupportedInterfaceOrientations' => [
      'UIInterfaceOrientationPortrait',
      'UIInterfaceOrientationLandscapeLeft',
      'UIInterfaceOrientationLandscapeRight',
    ],
    'NSMicrophoneUsageDescription' => '云游戏互动时需要开启麦克风'
  }
end

