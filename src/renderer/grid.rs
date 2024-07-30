use rand::random;
use wgpu::util::DeviceExt;

use bytemuck::{bytes_of, cast_slice, Pod, Zeroable};

#[repr(C)]
#[derive(Copy, Clone, Pod, Zeroable)]
pub struct GridDim {
    pub x: u32,
    pub y: u32,
}

pub struct Grid {
    texture: wgpu::Texture,
    u: wgpu::Buffer,
    v: wgpu::Buffer,
    dims_buf: wgpu::Buffer,
    pub dims: GridDim,

    pub draw_grid_bind_group: wgpu::BindGroup,
    pub draw_to_grid_bind_group: wgpu::BindGroup,
}

impl Grid {
    pub fn new_random(
        device: &wgpu::Device,
        draw_grid_layout: wgpu::BindGroupLayout,
        draw_to_grid_layout: wgpu::BindGroupLayout,
        width: u32,
        height: u32,
    ) -> Self {
        let dims = GridDim {
            x: width,
            y: height,
        };

        let dims_buf = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: None,
            usage: wgpu::BufferUsages::UNIFORM,
            contents: bytes_of(&dims),
        });

        let texture = device.create_texture(&wgpu::TextureDescriptor {
            label: None,
            size: wgpu::Extent3d {
                height,
                width,
                ..Default::default()
            },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba32Float,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT
                | wgpu::TextureUsages::TEXTURE_BINDING
                | wgpu::TextureUsages::STORAGE_BINDING,
            view_formats: &[],
        });

        let texture_view = texture.create_view(&wgpu::TextureViewDescriptor::default());

        let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
            label: Some("My Sampler"),
            address_mode_u: wgpu::AddressMode::ClampToEdge,
            address_mode_v: wgpu::AddressMode::ClampToEdge,
            address_mode_w: wgpu::AddressMode::ClampToEdge,
            mag_filter: wgpu::FilterMode::Nearest,
            min_filter: wgpu::FilterMode::Nearest,
            mipmap_filter: wgpu::FilterMode::Nearest,
            ..Default::default()
        });

        let mut random_data: Vec<f32> = Vec::with_capacity((width * height) as usize);
        for _ in 0..width * height {
            random_data.push(random());
        }

        let u = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Grid buffer"),
            usage: wgpu::BufferUsages::STORAGE,
            contents: cast_slice(&random_data),
        });

        let mut random_data: Vec<f32> = Vec::with_capacity((width * height) as usize);
        for _ in 0..width * height {
            random_data.push(random());
        }

        let v = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Grid buffer"),
            usage: wgpu::BufferUsages::STORAGE,
            contents: cast_slice(&random_data),
        });

        let draw_grid_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &draw_grid_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::TextureView(&texture_view),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::Sampler(&sampler),
                },
            ],
        });

        let draw_to_grid_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &draw_to_grid_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::TextureView(&texture_view),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                        buffer: &dims_buf,
                        offset: 0,
                        size: None,
                    }),
                },
                wgpu::BindGroupEntry {
                    binding: 2,
                    resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                        buffer: &u,
                        offset: 0,
                        size: None,
                    }),
                },
                wgpu::BindGroupEntry {
                    binding: 3,
                    resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                        buffer: &v,
                        offset: 0,
                        size: None,
                    }),
                },
            ],
        });

        Self {
            dims,
            dims_buf,
            texture,
            u,
            v,
            draw_to_grid_bind_group,
            draw_grid_bind_group,
        }
    }
}
