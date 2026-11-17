#pragma once

class ImageManager {
	ID3D11ShaderResourceView* image = nullptr;
public:
	void load( unsigned char* data, size_t data_size, ID3D11Device* device ) {
		if ( image )
			return;

        if (!data || !device || data_size > static_cast<size_t>(INT_MAX)) return;
        int width = 0, height = 0, channels = 0;
        unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(data_size),
            &width, &height, &channels, 4);
        if (!pixels) return;
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = pixels;
        init.SysMemPitch = static_cast<UINT>(width) * 4;
        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &init, &texture);
        stbi_image_free(pixels);
        if (SUCCEEDED(hr)) {
            device->CreateShaderResourceView(texture, nullptr, &image);
            texture->Release();
        }
	}

	ID3D11ShaderResourceView*& img( ) {
		return image;
	}

	static ImageManager& get( ) {
		static ImageManager s{ };
		return s;
	}
};