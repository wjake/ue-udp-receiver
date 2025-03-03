/**
 * Author: Jake Wright (github.com/wjake)
 * Date: 2025-02-24
 * File: UDPReceiver.h
 * Class: UUDPReceiver
 * Description: Actor component for receiving and processing UDP messages with event-driven functionality.
 * Dependencies: Networking.h, Sockets.h, UdpSocketReceiver.h
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Common/UdpSocketReceiver.h"
#include "UUDPReceiver.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogUDPReceiver, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageReceivedEvent, const FString&, ReceivedMessage);

UCLASS(ClassGroup = (Server), meta = (BlueprintSpawnableComponent))
class UUDPReceiver : public UActorComponent
{
    GENERATED_BODY()

public:
    UUDPReceiver();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    FString SocketName = TEXT("Socket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    FString IP = TEXT("0.0.0.0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    int32 Port = 65432;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Config")
    int32 BufferSize = 1024;

    UFUNCTION(BlueprintCallable, Category = "UDP")
    UUDPReceiver* StartReceiver();

    UFUNCTION(BlueprintCallable, Category = "UDP")
    UUDPReceiver* StopReceiver();

    UFUNCTION(BlueprintCallable, Category = "UDP Config")
    UUDPReceiver* UpdateConfig(const FString& NewIP, int32 NewPort, int32 NewBufferSize);

    UPROPERTY(BlueprintAssignable, Category = "UDP")
    FMessageReceivedEvent OnMessageReceivedEvent;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
private:
    FSocket* UdpSocket;
    TSharedPtr<FUdpSocketReceiver> UdpSocketReceiver;

    void OnMessageReceived(const TSharedPtr<FArrayReader, ESPMode::ThreadSafe>& Data, const FIPv4Endpoint& Endpoint);
};