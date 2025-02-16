//
//  TCGGamer.h
//  TCGSDK
//
//  Created by LyleYu on 2020/12/8.
//  Copyright © 2021 Tencent. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <tcgsdk/TCGSdkConst.h>
#import <tcgsdk/TCGCustomTransChannel.h>
#import <AVFoundation/AVFoundation.h>
#import <tcgsdk/TCGVideoFrame.h>
#import <tcgsdk/TCGAudioFrame.h>

@class TCGGameController;

@protocol TCGGamePlayerDelegate <NSObject>
@required
/*!
 * 本地资源初始化成功，必选
 * @param localSession 本地信息用于申请云端资源
 */
- (void)onInitSuccess:(NSString *)localSession;

/*!
 * 本地资源初始化失败，必选
 * @param errorCode 错误码
 * @param errorMsg 内部错误信息
 */
- (void)onInitFailure:(TCGErrorType)errorCode msg:(NSError *)errorMsg;

/*!
 * 与云端的连接异常，必选。(未成功建立连接，或连接因网络等原因异常断开)
 * @param errorCode 错误码
 * @param errorMsg 内部错误信息
 */
- (void)onConnectionFailure:(TCGErrorType)errorCode msg:(NSError *)errorMsg;

/*!
 * 与云端的连接异常，开始自动重连，必选。
 * @discussion 重连后之前连接的虚拟手柄与鼠标的信息需要重新设置。
 * @param reason 触发重连的原因
 */
- (void)onStartReConnectWithReason:(TCGErrorType)reason;

/*!
 * 视频流画面尺寸发生变化，(第一次连接成功后在画面显示之前也会回调)，必选。
 * @param videoSize 视频源宽高
 */
- (void)onVideoSizeChanged:(CGSize)videoSize;

/*!
 * (手游)视频画面的朝向发生改变，(进入游戏前也会回调)。
 * @param orientation 视频画面的朝向，UIInterfaceOrientationLandscapeRight 画面内容为横屏，UIInterfaceOrientationPortrait 画面内容为竖屏。
 * @discussion 注意这里的朝向是指画面里的内容是竖屏或横屏内容(可理解为云端手机的朝向)，而视频画面的宽高是不会发生改变的。
 */
- (void)onVideoOrientationChanged:(UIInterfaceOrientation)orientation;

/*!
 * 视频首帧出图(包括重连后出画面)，必选。
 */
- (void)onVideoShow;

@optional
/*!
 * 游戏进程启动成功，用于区别如账号登录窗口等游戏前置窗口（可选）。
 * 回调表明前置窗口已关闭，拉起了游戏主体窗口。(需后台配置来指定主体进程)
 */
- (void)onGameProcessRun;

// 暂未实现
//- (void)onReceiveTransChannel:(NSString *)name data:(NSData *)data;

@end

@protocol TCGGameArchiveDelegate <NSObject>
@optional
/*!
 * 下载存档的状态回调
 * @param status 状态：0: 加载存档成功； 1: 下载存档失败； 2: 校验存档失败；3: 解压存档失败； 4: 其他错误； 5: 下载中, loaded_szie表示下载的字节数
 * @param infoDic 存档处理的细节信息，字典key详情见官方wiki
 */
- (void)onLoadStatusChanged:(int)status allInfo:(NSDictionary *)infoDic;

/*!
 * 保存存档的状态回调
 * @param status 状态：0: 保存成功； 1: 保存失败； 2: 校验存档失败；3: 其他错误； 4: 上传中,saved_size表示保存的字节数； 5: 存档没找到; 6: 存档操作太频繁
 * @param infoDic 存档处理的细节信息，字典key详情见官方wiki
 */
- (void)onSaveStatusChanged:(int)status allInfo:(NSDictionary *)infoDic;

@end

/*!
 * sdk audioSession代理，实现并通过addTCGAudioSessionDelegate后。sdk中将不再操作AudioSession，而是将AudioSession相关操作通过该代理回调到APP
 * 处理，APP根据实际需要根据参数对AVAudioSession进行设置。
 */
@protocol TCGAudioSessionDelegate <NSObject>
@optional
- (BOOL)onSetCategory:(NSString *)category
        withOptions:(AVAudioSessionCategoryOptions)options
        error:(NSError **)outError;
- (BOOL)onSetMode:(NSString *)mode error:(NSError **)outError;
- (BOOL)onSetActive:(BOOL)active error:(NSError **)outError;
@end

#pragma mark --- 解码后视频帧接口 ---
@protocol TCGVideoSinkDelegate <NSObject>
@required
/*
  解码后的视频帧数据回调
 */
- (void)onRenderVideoFrame:(TCGVideoFrame *)frame;
@end

#pragma mark --- 音频数据回调接口 ---
@protocol TCGAudioSinkDelegate <NSObject>
@required
/**
 音频数据回调，如何使用参考TCGAudioFrame
 */
- (void)onAudioData:(TCGAudioFrame *)data;
@end

#pragma mark --- SDK云游基础类，提供云游能力 ---
@interface TCGGamePlayer : NSObject

@property(nonatomic, weak) id<TCGGamePlayerDelegate> delegate;
@property(nonatomic, weak) id<TCGGameArchiveDelegate> archiveDelegate;
@property(nonatomic, strong, readonly) TCGGameController *gameController;

/*!
 * 初始化本地资源，异步回调结果
 * @param params 选填，预留暂未启用
 * @param listener  必填，player的代理，监听关键事件的回调
 */
- (instancetype )initWithParams:(NSDictionary *)params andDelegate:(id<TCGGamePlayerDelegate>)listener;

/*!
 * 开始与云端建立连接
 * @param remoteSession 业务后台请求到的云端session信息
 * @param error 错误返回
 */
- (BOOL)startGameWithRemoteSession:(NSString *)remoteSession error:(NSError **)error;

/*!
 * 结束游戏，释放本地资源。
 * @discussion 云端资源在心跳断开90秒后自动释放，立即释放需业务后台主动调用接口。
 */
- (void)stopGame;

/*!
 * 重启云端的游戏应用程序
 */
- (void)restartGame;

/*!
 * 半退游戏，与云端的连接不主动断开，云端停止发送音视频数据。
 * @discussion 云端游戏进程不暂停。
 *  与云端的连接，只是不主动断开，若因网络等原因断开后不触发自动重连
 * @param doPause YES 半退游戏，NO 继续游戏
 */
- (void)pauseResumeGame:(BOOL)doPause;

/*!
 * 设置游戏视频的输出编码码率和帧率
 * @param minBitrate 范围[1*1000,15*1000]，单位Kbps
 * @param maxBitrate 范围[1*1000,15*1000]，单位Kbps
 * @param fps 范围[10,60]，单位帧;
 */
- (void)setStreamBitrateMix:(UInt32)minBitrate max:(UInt32)maxBitrate fps:(int)fps;

/*!
 * 设置远端声音的增益系数，默认为1.0保持源音量不变，2.0音量放大一倍
 * @param scale [0.0 ~ 10.0]
 * @discussion 过度放大声音，会引起失真
 */
- (void)setVolumeScale:(float)scale;

/*!
 * 设置本地麦克风的禁用与开启，默认关闭
 * @param enable YES 开启，NO 关闭
 */
- (void)setEnableLocalAudio:(BOOL)enable;

/*!
 * 设置连接超时时长，在连接开始前设置才有效，默认10秒
 * @param timeout 设置从连接远端开始到出画面的超时时长
 */
- (void)setConnectTimeout:(NSTimeInterval)timeout;

/*!
 * 获取视频渲染的图层
 * @return 视频视图 UIView
 */
- (UIView *)videoView;

/*!
 * 视频图层是否支持双指手势(放大、拖动)
 * @discussion 当前只支持放大视图，当尝试缩小时可恢复成最初的样子。
 *    双指拖动时，可将视图拖移出屏幕
 * @param isEnable YES, 支持双指操作(放大、拖动)；NO, 关闭手势识别。默认关闭
 */
- (void)setVideoViewEnablePinch:(BOOL)isEnable;

/*!
* 设置videoView在父视图上拖动的边缘间距，限制videoView不拖出视图。
* @param insets 指定边缘间距。默认(100, 50, 50, 100)
* @discussion 正值表示更接近矩形中心的边距，而负值表示远离中心的边距。
*/
- (void)setVideoViewInsets:(UIEdgeInsets)insets;

/*!
 * 将视频图层恢复成缩放/拖动前的状态
 */
- (void)resetVideoViewFrame;

/*!
 * 获取视频图层当前的放大系数
 * @return 放大系数， 1.0表示没有缩放
 */
- (CGFloat)videoViewScale;

/*!
 * (手游) 获取视频画面内容的朝向
 * @return 画面内容的朝向，UIInterfaceOrientationLandscapeRight 或 UIInterfaceOrientationPortrait
 */
- (UIInterfaceOrientation)videoOrientation;

/*!
 * 获取当前的统计信息
 * @return 信息字典NSDictionary
 */
- (NSDictionary *)currentStatisReport;

/*!
 * 修改云端桌面的分辨率，仅云端为PC Windows应用支持。修改后云端桌面的分辨率会改变为目标分辨率大小。
 * 如果客户端的gameView大小不变，修改分辨率后画面会有一定的拉伸/收缩。
 * @param width 需要修改到的目标云端桌面宽度
 * @param height 需要修改到的目标云端桌面高度
 */
- (void)setRemoteDesktopResolution:(int)width videoheight:(int)height;

/*!
 * 创建一个能与云端程序通讯的通道，最多允许存在三个通道。
 * @param remotePort 云端程序的UDP端口号
 * @param channelDelegate 通讯对象的代理，详情见TCGCustomTransChannelDelegate
 * @return TCGCustomTransChannel 通讯对象
 * @discussion 异步执行，通过TCGCustomTransChannelDelegate返回结果
 */
- (TCGCustomTransChannel *)openCustomTransChannel:(int)remotePort delegate:(id<TCGCustomTransChannelDelegate>)channelDelegate;

/*!
 * 开启自定义渲染模式，调用后sdk将不再向TCGGamePlayer.videoView上渲染画面
 * @param videoSink 待渲染视频帧的回调，详情见TCGVideoSinkDelegate
 */
- (void)setVideoSink:(id<TCGVideoSinkDelegate>)videoSink;

- (void)setAudioSink:(id<TCGAudioSinkDelegate>)audioSink;

/*!
 * 设置sdk audioSession代理，实现并通过addTCGAudioSessionDelegate后。sdk中将不再操作AudioSession，而是将AudioSession相关操作通过该代理回调到APP
 * 处理，APP根据实际需要根据参数对AVAudioSession进行设置。
 */
+ (void)setAudioSessionDelegate:(id<TCGAudioSessionDelegate>)delegate;

// 获取sdk audioSessionDelegate
+ (id<TCGAudioSessionDelegate>)audioSessionDelegate;
@end

#pragma mark --- 日志接口 ---
@protocol TCGLogDelegate <NSObject>
@required
/*!
 日志打印回调接口
 @param logLevel 日志打印级别
 @param log log
 */
- (void)logWithLevel:(TCGLogLevel)logLevel log:(NSString *)log;
@end

@interface TCGGamePlayer(Logger)
/*!
 * 设置日志回调接收者及过滤等级
 * @param logger 日志回调代理
 * @param minLevel  最低过滤日志等级
 */
+ (void)setLogger:(id<TCGLogDelegate>)logger withMinLevel:(TCGLogLevel)minLevel;

@end
