using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Main;
using TMPro;
using Unity.VisualScripting;

namespace Main
{
    [SaveLocation(SaveLocationAttribute.LocationType.Chunk)]
    public class Entity : SaveableBehaviour
    {
        public delegate void ChunkFallbackPositionEvent();
        public Vector2Int Chunk { get { return chunk; } }

        private Vector2Int chunk;

        private Transform entityTransform;

        public virtual bool IsChunkAssigned { get { return true; } }

        [SaveVar]
        public Vector3 EntityPosition { set { transform.position = value; } get { return entityTransform.position; } } // Avoid reading from as its slower

        [SaveVar]
        public Vector3 EntityRotation { set { transform.rotation = Quaternion.Euler(value); } get { return entityTransform.eulerAngles; } }

        public ChunkFallbackPositionEvent OnChunkFallbackPosition { get; set; }

        public bool IsUnsaving { get; private set; } = false;

        private void Awake()
        {
            entityTransform = transform;
        }

        protected virtual void Start()
        {
            // I know its weird im running this twice
            // but entityTransform will be null, and i want it done in awake for external things
            // Idk dawg too tired to fix it rn
            entityTransform = transform;

            chunk = Map.NearestChunk(entityTransform.position);

            Chunks.EntityHasChanged(this, chunk);
        }


        // Should be used when modifying any entity's location, cache the transform for speed and are Chunk Location Safe
        #region TRANSFORMATIONS

        public void SetPosition(Vector3 position)
        {
            entityTransform.position = position;
            OnPositionUpdated(position);
        }
        public void SetPosition(float x, float y, float z) => SetPosition(new Vector3(x, y, z));
        public void SetRotation(Quaternion rotation) => entityTransform.rotation = rotation;
        public void SetRotation(Vector3 rotation) => entityTransform.rotation = Quaternion.Euler(rotation);
        public void SetRotation(float x, float y, float z) => SetRotation(new Vector3(x, y, z));
        public void SetScale(Vector3 scale) => entityTransform.localScale = scale;
        public void SetScale(float x, float y, float z) => SetScale(new Vector3(x, y, z));

        #endregion

        public void SetUnsaveable()
        {
            IsUnsaving = true;
        }

        public void OnPositionUpdated(Vector3 newPosition)
        {
            if (!IsChunkAssigned) return;
            Vector2Int oldChunk = chunk;
            chunk = Map.NearestChunk(newPosition);
            if (oldChunk != chunk)
            {
                Chunks.EntityHasChanged(this, oldChunk, chunk);
            }
        }

        // Entities should track their current chunk when the chunk is changed
        // If they're in a changed chunk they should disable and save their state into the map
        // with an "CHUNK: Saves(Entities(EntitiesDatas)) adn whatnot layout so they can be near
        // seamlessly spawned back in
    }
}